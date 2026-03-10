## Alarm sensor-node, with Task scheduler → ![](https://img.shields.io/badge/status-under%20development-8A2BE2)
#### Overview
This is a modular component of a larger IoT system (Smart Hub) consisting of multiple nodes.

This repo handles all sensor data for an alarm system, utilizing an Arduino Uno R4 WiFi with a bespoke non-blocking task scheduler to manage concurrent sensor polling and network communication. Sensor data is distributed to neighboring nodes via Wi-Fi using the MQTT protocol, and via BLE.


...more info is coming

#### Architecture
...

#### Features
...

#### Hardware
* MCU: Arduino Uno R4 WiFi
* Sensors:

        → DHT11 : indoor temp + indoor humidity
        → MQ2 : Gas sensor (detecting fire)
        → DS18B20 : Temp sensor (detecting fire)
        → PIR : Motion detector
        → Reed : Magnet sensor (detect open doors/windows)
        → Rain-sensor : Detect water leak
