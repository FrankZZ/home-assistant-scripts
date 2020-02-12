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


Parts list:
---

- €1.64 [Pack of 2 enclosures](https://www.banggood.com/2pcs-75-x-54-x-27mm-DIY-Plastic-Project-Housing-Electronic-Junction-Case-Power-Supply-Box-p-1063302.html)
- €11.61 [Pack of 5 Arduino Pro Mini 3.3v 8MHz](https://www.banggood.com/5Pcs-3_3V-8MHz-ATmega328P-AU-Pro-Mini-Microcontroller-Board-For-Arduino-p-980292.html)
- €1.66 [Pack of 20 screw terminals](https://www.banggood.com/20pcs-2-Pin-Plug-In-Screw-Terminal-Block-Connector-5_08mm-Pitch-p-993197.html)
- €8.78  [Pack of 2 LiFePO4 3.2v 1800mAh Batteries](https://www.aliexpress.com/item/2pcs-Soshine-1800mAh-3-2V-LiFePO4-Battery-18650-Rechargeable-Battery-with-Protected-PCB-Portable-Battery-Storage/32818927889.html)
- €7.02 [Pack of 40 double side PCBs (1x 30x60mm used for this node)](https://www.banggood.com/40pcs-FR-4-Double-Side-Prototype-PCB-Printed-Circuit-Board-p-995732.html)
- €6.43 [200m of 0.55mm electronic wire](https://www.banggood.com/200m-0_55mm-8-Color-Circuit-Board-Single-Core-Tinned-Copper-Electronic-Wire-Fly-Wire-Jumper-Cable-Dupont-Wire-p-1106659.html)
- €6.36 [Charger compatible with LiFePO4 3.2V](https://www.aliexpress.com/item/LiitoKala-Lii-100B-18650-Battery-Charger-For-26650-16340-CR123-LiFePO4-1-2V-Ni-MH-Ni/32809893759.html)
- €12.08 [Pack of 10 AM312 PIR Motion Sensors](https://www.aliexpress.com/item/10-pcs-Mini-IR-Pyroelectric-Infrared-PIR-Motion-Human-Sensor-Automatic-Detector-Module-AM312-Sensor-DC/32831139174.html)
- €1.60 [FT232RL USB Serial Adapter](https://www.aliexpress.com/item/1pc-Basic-Breakout-Board-For-FTDI-FT232RL-USB-to-Serial-IC-For-Hot-Top-Sale/32648158894.html)
- €2.36 [BME280 Temperature, Humidity and Barometric Pressure Sensor](https://www.aliexpress.com/item/1PCS-Digital-Temperature-Humidity-Barometric-Pressure-Sensor-Module-Breakout-BME280-SPI-and-I2C-interface/32795463531.html)
- €6.32 [Pack of 10 NRF24L01+ 2.4GHz Modules](https://www.aliexpress.com/item/10pcs-Wireless-Transceiver-For-Arduino-NRF24L01-2-4GHz-Antenna-Module-For-Microcontroll-DropShipping/32599246023.html)
- €0.82 [BH1750 Lux Sensor Module](https://www.aliexpress.com/item/GY-302-BH1750-BH1750FVI-light-intensity-illumination-module-for-arduino-3V-5V/32341898423.html)

To save costs I have ordered some bulk quantities, because I want to create multiple nodes anyway.

Based on taking 1 piece of all components in this list, the total price for this node is €12,81
