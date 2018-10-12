#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define CYCLES_PER_KWH 375
int PULSE_LEFT = LOW;
int PULSE_RIGHT = HIGH;

int LEFT_SENSOR_PIN = D5;
int RIGHT_SENSOR_PIN = D6;

int W = 0;                     // variable to store value watt
unsigned long starttime;       // variable to store starttime of puls
unsigned long duration;        // variable to store duration value
volatile int wattpulse = LOW;

volatile int pulseDirection = -1;

int counter = 0;

long debouncing_time = 20; //Debouncing Time in Milliseconds
volatile unsigned long last_micros_left;
volatile unsigned long last_micros_right;
volatile unsigned long last_micros;

const char* ssid = "...";
const char* password = "...";

ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite(LED_BUILTIN, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(LED_BUILTIN, 0);
}

void handleNotFound(){
  digitalWrite(LED_BUILTIN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, 0);
}

void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Sensors for kWh
  attachInterrupt(LEFT_SENSOR_PIN, debounceInterruptLeft, RISING);
  attachInterrupt(RIGHT_SENSOR_PIN, debounceInterruptRight, RISING);
  starttime = millis();

}

void loop() {
  if ((wattpulse == HIGH)/* && ((counter % 2) != 0)*/) {
    duration = millis() - starttime;
    starttime = millis();
    W = 3600000000 / (CYCLES_PER_KWH * duration); // calculate current power usage
    if (pulseDirection == PULSE_LEFT) {
      W = -W;
    }
    Serial.println(W);
    wattpulse = LOW;
  }

  server.handleClient();
}

void watt() {
  wattpulse = HIGH;
  counter++;
}

void debounceInterruptRight() {
  if((long)(micros() - last_micros_right) >= debouncing_time * 1000) {

    if (pulseDirection == PULSE_RIGHT) {

      if (last_micros_left > last_micros) {
        watt();
      }

      last_micros = last_micros_right;
    } else if (last_micros == last_micros_left) {
      // the left sensor was the last to sense a pulse, so its turning right now
      Serial.println("PULSE RIGHT");
      pulseDirection = PULSE_RIGHT;
    }

    last_micros_right = micros();
  }
}

void debounceInterruptLeft() {
  if((long)(micros() - last_micros_left) >= debouncing_time * 1000) {

    if (pulseDirection == PULSE_LEFT) {

      if (last_micros_right > last_micros) {
        watt();
      }

      last_micros = last_micros_left;
    } else if (last_micros == last_micros_right) {
      // the right sensor was the last to sense a pulse, so its turning left now
      Serial.println("PULSE LEFT");
      pulseDirection = PULSE_LEFT;
    }

    last_micros_left = micros();

  }
}

