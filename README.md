# Alarm sensor node, with RTOS → ![](https://img.shields.io/badge/status-under%20development-8A2BE2)

This is a the sensor-node part, of the **Home Hub** project.

This repository manages all sensor data, running on an Arduino Uno R4 WiFi. The system is designed to be deterministic with non-blocking functions for maximum reliability. The communication to the Broker (RBP Zero) & the Gateway (ESP32) occurs via BLE and WIFI (MQTT).

---

## Architecture - RTOS & Tasks

The system utilizes Hardware Interrupts for immediate sensor detection and FreeRTOS for task orchestration and Binary Semaphores.
* Priority 3 (High): Critical alarm events (PIR/Reed/MQ2/DS18B20) via hardware interrupts & semaphores.
* Priority 2 (Medium): Network communication (WiFi/MQTT/BLE).
* Priority 1 (Low): System monitoring (Temp/Water leak).
* Software Timer: Used for a real-time LED status indication, without the memory cost of a dedicated task.

---
## Alarm-procedure

When an alarm is detected - at fire or intrution:
1) The system notice the alarm, and update the state machine
2) The alarm info is stored in a packed struct of 5 bytes, "AlarmInfo"
   1) Timestamp is added (unix time)
   2) Alarmtype is added (what kind of alarm)
3) The alarm info is sending three times to a queue (AlarmQueue)
4) The xNetworkTask wake up at queue - and send the alarm to BLE.

---
## Comminucation: BLE & MQTT 

BLE (Tx) - `Indicate with handshake`:
  * Heartbet: Every 5s 
  * Send critical alarms, as packages.

MQTT (Tx):
* Heartbet: Every 10s
* Has Will & Testament (LWT)
  * If the sensor node stops sending heartbeats, an 'OFFLINE' status will be published by the Broker.
* Send info every 30s: temp/humidity & ( soon water leak )

---
## Upcomming

* Implement & integrate other sensors (MQ2, PIR, Water-leak)
* BLE (Rx) - Receive and handle alarm state from ESP32 (GW)
* Add time-zone (ezTime ?)

---
## Hardware
* MCU: Arduino Uno R4 WiFi

* Sensor:

| Sensors       | Detect                        | PIN @ Arduino | Transmits to     | Implemented  | External Interrupts | Triggering State |
| ------------- |:-----------------------------:|:-------------:|:----------------:|:------------:|:-------------------:|:----------------:|
| DHT11         | Indoor: temp + humidity       | D2 (P104)     |  Broker @ MQTT   | Yes          |                     | -                |
| DS18B20       | Fire @ Temp (pull-up needed)  | D5 (P107)     |  ESP32 @ BLE     | Yes          |                     | [Always]         |
| MQ2           | Fire @ Gas                    |  -            |  ESP32 @ BLE     |              | Yes                 | [Always]         |
| Rain-sensor   | Water leak                    |  -            | *(Broker @ MQTT)*  |              |                     | [Always]         |
| PIR           | Motion                        | D4 (P106)     |  ESP32 @ BLE     |              | Yes                 | Armed away       |
| Reed          | Open door or window           | D3 (P105)     |  ESP32 @ BLE     | Yes          | Yes                 | Armed away/home  |

---
## More info ####

* *RTC (Time stamp)*
  * The clock is synchronized via WiFi on startup
  * Every alarm event has an time stamp

---
## Setup - Info

When an alarm is detected - at fire or intrution:
1) The system notice the alarm, and update the state machine
2) The alarm info is stored in a packed struct of 5 bytes, "AlarmInfo"
   1) Timestamp is added (unix time)
   2) Alarmtype is added (what kind of alarm)
3) The alarm info is sending three times to a queue (AlarmQueue)
4) The xNetworkTask wake up at queue - and send the alarm to BLE.

---
## Comminucation: BLE & MQTT 

BLE (Tx) - `Indicate with handshake`:
  * Heartbet: Every 5s 
  * Send critical alarms, as packages.

MQTT (Tx):
* Heartbet: Every 10s
* Has Will & Testament (LWT)
  * If the sensor node stops sending heartbeats, an 'OFFLINE' status will be published by the Broker.
* Send info every 30s: temp/humidity & ( soon water leak )

---
## Changelog
### [2026-03-31] - MQTT & NTP Updates
- Added TLS to current MQTT/WiFi management.
- Switched time synch to local Access Point(Zero) to avoid "homerouter" config dependency.
- Added `build_flags` for MQTT Broker and NTP Server configuration in `platformio.ini`.

* TLS
    -IMPORTANT!-
    To circumvent unstable packet reassembly(by sensornode), server.crt may NOT exceed 1,460 bytes 
    and ca.crt must be excluded from handshake payload! (se hemmahubb/guide)

  To manually add your certificate with proper format, use the helper-tool to avoid SSL errors; 

    1. Inspect->copy->paste ca.crt(root certificate) from your broker(Zero) into the converter/formatter below:
    https://unreeeal.github.io/ssl_esp.html
    2. Create ./src/certificate.cpp and paste the helper-tool output in the file.

* Time synch
    To use the new timesync your AP is required to have a timeserver, ex. ntpsec(se hemmahubb/guide). 
    If you prefer using WiFi.getTime(), just comment out the new initTime() and de-comment the old one.

