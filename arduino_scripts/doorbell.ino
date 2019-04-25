int DOORBELL_PRESSED_SENSOR_PIN = D8;
unsigned int lastDoorbellPushed = -1;
unsigned int heartbeatDuration = 55 * 1000; // in ms

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define wifi_ssid "xxx"
#define wifi_password "xxx"

#define mqtt_server "xxx"
#define mqtt_clientid "ESP8266Doorbell"

// Uncomment following lines if you are using user/pass
// #define mqtt_user ""
// #define mqtt_password ""

#define ota_password "xxx"

#define doorbell_topic "sensor/doorbell"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  if (!client.connected()) {
    reconnect();
  }

  pinMode(DOORBELL_PRESSED_SENSOR_PIN, INPUT);
  lastDoorbellPushed = digitalRead(DOORBELL_PRESSED_SENSOR_PIN); // can be also released pin
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
  //ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("DOORBELL");

  // No authentication by default
  ArduinoOTA.setPassword(ota_password);

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
#ifdef mqtt_user
    if (client.connect(mqtt_clientid, mqtt_user, mqtt_password)) {
#else
    if (client.connect(mqtt_clientid)) {
#endif
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

unsigned long lastMsg = 0;
unsigned long lastHeartbeat = 0;
unsigned long doorbellPushedTicks = 0;
void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  unsigned int currentDoorbellPushed = digitalRead(DOORBELL_PRESSED_SENSOR_PIN);

  if (currentDoorbellPushed == HIGH) {
      doorbellPushedTicks++;
  }
  else {
      doorbellPushedTicks = 0;
  }

  if (now - lastMsg > (10*1000) && lastDoorbellPushed != currentDoorbellPushed && (doorbellPushedTicks > 20 || lastDoorbellPushed == HIGH)) {
    lastMsg = now;
    lastHeartbeat = now;
    lastDoorbellPushed = currentDoorbellPushed;
    doorbellPushedTicks = 0;
    publishDoorbell(currentDoorbellPushed);

  } else if (now - lastHeartbeat > heartbeatDuration) {
    lastHeartbeat = now;
    publishDoorbell(lastDoorbellPushed);
  }
}

void publishDoorbell(unsigned int currentDoorbellPushed) {
  if (currentDoorbellPushed == HIGH) {
      // lets push a message to MQTT
      Serial.println("publishing pressed");
      client.publish(doorbell_topic, String("pressed").c_str(), true);
    } else if (currentDoorbellPushed == LOW) {
      // lets push a message to MQTT
      Serial.println("publishing not_pressed");
      client.publish(doorbell_topic, String("not_pressed").c_str(), true);
    }
}
