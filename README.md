## Alarm sensor node, with RTOS → ![](https://img.shields.io/badge/status-under%20development-8A2BE2)
#### Overview
This is a modular component of a larger IoT system (Smart Hub) consisting of multiple nodes.

This repository manages sensor data for an alarm system running on an Arduino Uno R4 WiFi. The system is designed to be deterministic with non-blocking functions for maximum reliability.

#### Architecture - Interrupts & RTOS

The system utilizes Hardware Interrupts for immediate sensor detection and FreeRTOS for task orchestration and Binary Semaphores.
* Priority 3 (High): Critical alarm events (PIR/Reed) via hardware interrupts & semaphores.
* Priority 2 (Medium): System monitoring (Temp/Water/LED).
* Priority 1 (Low): Network communication (WiFi/MQTT).

#### Setup - Info

1) Update your WIFI: SSID + Password → plattformio.ini
2) Update your Broker/Zero IP adress → mqtt_client.cpp
3) When WIFI is disconnected → solid red light
4) When system is up and running → red blink
5) The system is now sending MQTT when ready!

#### Upcomming

* Implement & integrate other sensors (DS18B20 etc)
* BLE comminucation
* 

#### Hardware
* MCU: Arduino Uno R4 WiFi

* Sensor:

| Sensors       | Detect                        | PIN @ Arduino | Transmits to     | Implemented  | External Interrupts |
| ------------- |:-----------------------------:|:-------------:|:----------------:|:------------:|:-------------------:|
| DHT11         | Indoor: temp + humidity       | D2 (P104)     |  Broker @ MQTT   | Yes          |                     |
| DS18B20       | Temp, fire  (pull-up needed)  | D5 (P107)     | *(Broker @ MQTT)*  |              |                     |
| MQ2           | Gas, fire                     |  -            | *(Broker @ MQTT)*  |              |                     |
| Rain-sensor   | Water leak                    |  -            | *(Broker @ MQTT)*  |              |                     |
| PIR           | Motion                        | D4 (P106)     | *(ESP32 @ BLE)*    |              | Yes                 |
| Reed          | Open door or window           | D3 (P105)     | *(ESP32 @ BLE)*    | Yes          | Yes                 |