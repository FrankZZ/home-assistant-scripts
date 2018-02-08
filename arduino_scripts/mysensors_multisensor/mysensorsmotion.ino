/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Motion Sensor example using HC-SR501
 * http://www.mysensors.org/build/motion
 *
 */

#include <BH1750.h>

#include <Wire.h>

// Enable debug prints
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#define MY_DEFAULT_LED_BLINK_PERIOD 0
#include <MySensors.h>

#include <BME280_MOD-1022.h> // I2C
BH1750 lightSensor;  // I2C

//unsigned long SLEEP_TIME = 10000; // Sleep time between reports (in milliseconds)
unsigned long SLEEP_TIME = 300000; // Sleep time between reports (in milliseconds)
//unsigned long SLEEP_TIME = 120000; // Sleep time between reports (in milliseconds)
#define DIGITAL_INPUT_MOTION 3   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define CHILD_ID_MOTION 1   // Id of the sensor child
#define CHILD_ID_TEMP 2   // Id of the sensor child
#define CHILD_ID_BARO 3   // Id of the sensor child
#define CHILD_ID_HUM 4   // Id of the sensor child
#define CHILD_ID_LUX 5   // Id of the sensor child

bool lastValue = false;
int8_t wakeupReason = -42;

float lastTC, lastH, lastP;
uint16_t lastLX;

// Initialize motion message
MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);
MyMessage msgLight(CHILD_ID_LUX, V_LEVEL);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgPressure(CHILD_ID_BARO, V_PRESSURE);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);

void setup()
{
  pinMode(DIGITAL_INPUT_MOTION, INPUT);      // sets the motion sensor digital pin as input
  Wire.begin();
  lightSensor.begin(BH1750::ONE_TIME_LOW_RES_MODE);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Multi-Sensor", "2.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_MOTION, S_MOTION);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_BARO, S_BARO);
  present(CHILD_ID_LUX, S_LIGHT_LEVEL);
}

void loop()
{
  if (wakeupReason == 1) { // reason - motion
    if (sendMotion()) { // if tripped=true also send other sensor values
      sendTemperatureEtc();
    }
  } else if (wakeupReason < 0) { // reason - SLEEP_TIME exceeded
    sendTemperatureEtc();
  }
  #ifdef MY_DEBUG
    Serial.println("wakeupreason " + String(wakeupReason));
  #endif MY_DEBUG
  wakeupReason = sleep(digitalPinToInterrupt(DIGITAL_INPUT_MOTION), CHANGE, SLEEP_TIME);
}

bool sendMotion() {
  // Read digital motion value
    bool tripped = (digitalRead(DIGITAL_INPUT_MOTION) == HIGH);
    if (tripped != lastValue) {
      lastValue = tripped;
      send(msgMotion.set(tripped? "1" : "0"));  // Send tripped value to gw
      return tripped;
    }
    return false;
}

void bme280_forcedSample() {
  // need to read the NVM compensation parameters
  BME280.readCompensationParams();
  
  BME280.writeOversamplingPressure(os8x);
  BME280.writeOversamplingTemperature(os8x);
  BME280.writeOversamplingHumidity(os8x);
  BME280.writeMode(smForced);

  #ifdef MY_DEBUG
  Serial.println("BME280 Forced Sample Reading");
  Serial.print("Measuring");
  #endif
  
  while (BME280.isMeasuring()) {
  #ifdef MY_DEBUG
    Serial.print(".");
  #endif
    delay(50);
  }
#ifdef MY_DEBUG
  Serial.println("Done!");
#endif

  // read out the data - must do this before calling the getxxxxx routines
  BME280.readMeasurements();
}

void sendTemperatureEtc() {
  bme280_forcedSample();
  float TF, TC, P, H;
  uint16_t LX;

  TC = BME280.getTemperature();
  H = BME280.getHumidity();
  P = BME280.getPressure();

  LX = lightSensor.readLightLevel();

  TC = round(TC * 10) / 10.0;
  H = round(H * 10) / 10.0;
  P = round(P * 10) / 10.0;
  LX = round(LX * 10) / 10.0;

  if (lastTC != TC) {
    send(msgTemp.set(TC, 1));  
  }
  
  if (lastH != H) {
    send(msgHum.set(H, 1));
  }

  if (lastP != P) {
    send(msgPressure.set(P, 1));
  }

  if (lastLX != LX) {
    send(msgLight.set(LX));
  }

  lastTC = TC;
  lastH = H;
  lastP = P;
  lastLX = LX;

#ifdef MY_DEBUG
  Serial.println("TC " + String(lastTC));
  Serial.println("H " + String(lastH));
  Serial.println("P " + String(lastP));
  Serial.println("LX " + String(lastLX));
#endif
}

