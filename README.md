## Alarm sensor node, with custom task scheduler → ![](https://img.shields.io/badge/status-under%20development-8A2BE2)
#### Overview
This is a modular component of a larger IoT system (Smart Hub) consisting of multiple nodes.

This repo handles all sensor data for an alarm system, utilizing an Arduino Uno R4 WiFi with a bespoke non-blocking task scheduler to manage concurrent sensor polling and network communication. Sensor data is distributed to neighboring nodes via Wi-Fi using the MQTT protocol, and via BLE.

#### Architecture
-

#### TO DO → Info / Setup

* Update your WIFI: SSID + Password → plattformio.ini
* Update your Broker/Zero IP adress → mqtt_client.cpp

###### *Good to know:*
* WiFi must be working to start-up right now..
* Startup time ~20s → when system is up and running : red blink on the led matrix

#### Hardware
* MCU: Arduino Uno R4 WiFi

* Sensor:

| Sensors       | Detect                        | PIN @ Arduino | Transmits to     | Implemented  |
| ------------- |:-----------------------------:|:-------------:|:----------------:| ------------:|
| DHT11         | Indoor: temp + humidity       | D2 (P104)     |  Broker @ MQTT   | Yes          |
| DS18B20       | Temp, fire  (pull-up needed)  | D5 (P107)     | *(Broker @ MQTT)*  | Yes          |
| MQ2           | Gas, fire                     |  -            | *(Broker @ MQTT)*  |              |
| Rain-sensor   | Water leak                    |  -            | *(Broker @ MQTT)*  |              |
| PIR           | Motion                        |  -            | *(ESP32 @ BLE)*    |              |
| Reed          | Open door / window            |  -            | *(ESP32 @ BLE)*    |              |