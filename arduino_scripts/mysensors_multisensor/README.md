MySensors 2.x Multi-sensor
======

This Arduino sketch is built to support multiple sensors via MySensors

Sensors:
- BH1750 Light Intensity in lux
- BME280 Temperature, Humidity and Barometric Pressure
- AM312 Motion Sensor PIR or any other PIR sensor with HIGH trigger

This sketch is tested on an Arduino Pro Mini 3.3v with the Remove Power LED and Remove 3.3v Regulator mod. I am running it on a Soshine 3.2v LiFEPO4 battery.

In power down sleep mode this sketch with sensors uses around 21µA.

Between the PIR trigger pin rising and falling the Arduino Pro Mini goes back to sleep.

When the PIR trigger pin is rising, we also sense the light and temperature related values. When changed, they are sent to MySensors gateway.
