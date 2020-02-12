#define CYCLES_PER_KWH 375
int PULSE_LEFT = LOW;
int PULSE_RIGHT = HIGH;

int LEFT_SENSOR_PIN = D6;
int RIGHT_SENSOR_PIN = D5;

int W = 0;                         // variable to store value watt
volatile unsigned long starttime;  // variable to store starttime of puls
unsigned long duration = 0;        // variable to store duration value
volatile int wattpulse = LOW;

volatile int pulseDirection = -1;
int lastPulseDirection = -1;

volatile int counter = 0;

long debouncing_time = 100;         //Debouncing Time in Milliseconds
volatile unsigned long last_millis_left;
volatile unsigned long last_millis_right;
volatile unsigned long last_millis = 0;

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define wifi_ssid "..."
#define wifi_password "..."

#define mqtt_server "..."
#define mqtt_user ""
#define mqtt_password ""

#define watt_topic "sensor/power_meter/watt"
#define direction_topic "sensor/power_meter/direction"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  if (!client.connected()) {
    reconnect();
  }

  client.publish(direction_topic, String("SENSING").c_str(), true);

  attachInterrupt(LEFT_SENSOR_PIN, debounceInterruptLeft, RISING);
  attachInterrupt(RIGHT_SENSOR_PIN, debounceInterruptRight, RISING);
  starttime = millis();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

    // Port defaults to 8266
//     ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
     ArduinoOTA.setHostname("...");

    // No authentication by default
     ArduinoOTA.setPassword("...");

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 200) {
    lastMsg = now;

    if ((wattpulse == HIGH)/* && ((counter % 2) != 0)*/) {
      duration = now - starttime;
      starttime = now;
      W = 3600000000 / (CYCLES_PER_KWH * duration); // calculate current power usage
      if (pulseDirection == PULSE_LEFT) {
        W = -W;
      }
      if (lastPulseDirection != pulseDirection) {
        lastPulseDirection = pulseDirection;
        if (pulseDirection == PULSE_LEFT) {
          client.publish(direction_topic, String("LEFT").c_str(), true);
        } else if (pulseDirection == PULSE_RIGHT) {
          client.publish(direction_topic, String("RIGHT").c_str(), true);
        }
      }
      Serial.println(W);
      client.publish(watt_topic, String(W).c_str(), true);
      wattpulse = LOW;
    } else {
      if (duration > 0 && (now - last_millis) >= (duration*2)) {
        // we waited for twice the amount of time that it took to rotate disk once
        // this means we are using at least only half of the power now
        duration = now - last_millis;
        int W2 = (3600000000 / (CYCLES_PER_KWH * duration)); // calculate current power usage (no pulse yet, send half value)
        if (pulseDirection == PULSE_LEFT) {
          W2 = -W2;
        }
        client.publish(direction_topic, String("HALFED").c_str(), true);
        pulseDirection = -1;
        client.publish(watt_topic, String(W2).c_str(), true);
      }
    }
  }
}

void watt() {
  wattpulse = HIGH;
  counter++;
}

void debounceInterruptRight() {
  volatile unsigned long currMillis = millis();
  if((long)(currMillis - last_millis_right) >= debouncing_time) {
    if (pulseDirection == PULSE_RIGHT) {
      if ((currMillis - last_millis_left) < (currMillis - last_millis)) {
        watt();
        last_millis_right = currMillis;
        last_millis = last_millis_right;
      }
    } else if ((currMillis - last_millis_left) < (currMillis - last_millis)) {
      // the left sensor was the last to sense a pulse, so its turning right now
      Serial.println("PULSE RIGHT");
      pulseDirection = PULSE_RIGHT;

      last_millis_right = currMillis;
      last_millis = last_millis_right;
      starttime = currMillis;
      client.publish(direction_topic, String("RIGHT").c_str(), true);

    } else {
      last_millis_right = currMillis;
    }
  }
}

void debounceInterruptLeft() {
  volatile unsigned long currMillis = millis();
  if((long)(currMillis - last_millis_left) >= debouncing_time) {

    if (pulseDirection == PULSE_LEFT) {
      if ((currMillis - last_millis_right) < (currMillis - last_millis)) {
        watt();
        last_millis_left = currMillis;
        last_millis = last_millis_left;
      }
    } else if ((currMillis - last_millis_right) < (currMillis - last_millis)) {
      // the right sensor was the last to sense a pulse, so its turning left now
      Serial.println("PULSE LEFT");
      pulseDirection = PULSE_LEFT;
      last_millis_left = currMillis;
      last_millis = last_millis_left;
      starttime = currMillis;
      client.publish(direction_topic, String("LEFT").c_str(), true);

    } else {
      last_millis_left = currMillis;
    }
  }
}
