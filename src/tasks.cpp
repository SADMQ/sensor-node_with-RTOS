// --- definierar HUR (sekvensen) och VAD som ska köras --- 
#include <stdio.h>
#include <Arduino.h>
#include <RTC.h>
#include <WiFiUdp.h> // udp
#include <NTPClient.h> // NTP clock
#include "tasks.h"
#include "alarm.h"
#include "sensor_dht11.h"
#include "sensor_ds18b20.h"
#include "indicateStatus.h"
#include "wifi_manager.h"
#include "mqtt_client.h"
#include "sensor_motion.h"
#include "sensor_reed.h"
#include "ble_manager.h"

void initComponents(){
    initCredentials();
    initDHT();
    initDS18B20();
    //initPIR();
    //initMQ2();
    initReed();
    initMatrix();
}

// ställer klockan om broker saknas (denna borde kunnas kan tas bort sen?)
int initTimeWiFi(){
    unsigned long epoch = WiFi.getTime(); 
  
    if (epoch != 0) {
        RTCTime startTime(epoch);
        RTC.setTime(startTime); // Nu är klockan ställd!
        Serial.println("RTC: Clock synchronized!");
        return true;
   }
        return false;
}

int initTimeNTP(){ 
        RTC.begin();
        WiFiUDP ntpUDP;
        NTPClient timeClient(ntpUDP, ZeroIP, 0, 60000); // Hämtar tid från Pi Accesspunkt

        //int attempts = 0;

        timeClient.begin();

        // Single loop: forceUpdate() returns true only on a successful packet receipt
        timeClient.forceUpdate(); //&& attempts++ < 5) {
            //delay(1000); 
        //}

        unsigned long epoch = timeClient.getEpochTime();
        
        if (epoch > 10000000) { 
            RTCTime rtcTime(epoch);
            RTC.setTime(rtcTime);
            Serial.print("RTC Synced: "); Serial.println(epoch);
            node.NTCsynced = true;
            return true;
        }

        Serial.println("RTC: NTP Failed");
        return false;
    }

// RTOS: Task - hanterar LARM, inbrott+brand (PIR+REED + GAS+TEMP)
// Händelsestyrd
void vAlarmTask(void *Params){
    // allt här körs EN gång
    for (;;){
        // väntar på given semafor - dvs. ett HW-interrupt, 
        BaseType_t xResult = xSemaphoreTake(xAlarmSemaphore, pdMS_TO_TICKS(2000)); // portMAX_DELAY - DS18B20 här, verkar sänka mqtt.. one-wire?

            // kör endast vid semaphore
            if (xResult){
                if (node.sensors.HWEvent_motionDetect){
                node.sensors.motionDetect = true;
                // BLE eller MQTT? - MQTT för lagring?
                node.sensors.HWEvent_motionDetect = false;
                }

                if (node.sensors.HWEvent_reedSensor1){
                node.sensors.reedSensor1 = true;
                // BLE eller MQTT? - MQTT för lagring?
                node.sensors.HWEvent_reedSensor1 = false;
                }
            } else {
                // går ENDAST på tidsintevall - oberoende semaphore, ~2000ms. 
                getDS18B20data();
                }

        checkAlarmStatus(); // Checka om något larm är triggat.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


// RTOS: Task - hanterar nätverk -> WIFI, MQTT & (BLE..)
// Händelsestyrd & tidsstyrd
void vNetworkTask(void *Params){
    // körs bara EN gång
    initWiFi();
    
    Serial.println("WiFi: Init - Complete!");

    for (;;){
        manageWiFi();
        if (wifiIsConnected()){

            // sätter tiden @ WIFI
            if (!node.timeIsSet){
                node.timeIsSet = initTimeWiFi();
            }

            manageMQTT();

            // synca tiden mot broker
            if (!node.NTCsynced && node.connectionStatus.mqttIsActive){
                node.NTCsynced = initTimeNTP();
            }
        }
        vTaskDelay(5000);
    }
}

// RTOS: Task - hanterar låg prio sensorer & status LED
// Tidsstyrd
void vSystemMonitorTask(void *Params){
    // Allt här körs EN gång

    for (;;){
        readLowPrioSensors();         
        vTaskDelay(pdMS_TO_TICKS(30000)); // pausa task, 30s
    }
}


int readLowPrioSensors(){
    static int currentSensor = READING_DHT; // static -> sätts endast EN gång (init)

    if (getDHTData()){
        Serial.print("DHT11: ");
        Serial.print(node.sensors.indoorTemp, 1); // En decimal
        Serial.print(" °C | Humidity: ");
        Serial.print(node.sensors.indoorHumidity, 1);
        Serial.println(" %");
    } else {
        Serial.print("DHT11: ERROR");
    }


    Serial.println("WL: No water leak..\n"); 
    // kolla water leak sensorn här..
}


void vBLETask(void* Params){
    // skicka ble från queue 
    initBLE();
    const AlarmInfo heartbeat = {NONE, 0}; 
    AlarmInfo alarmInfoToSend;
    Serial.println("BLE: Init - Complete!");

    for (;;){
        BaseType_t xResult = xQueueReceive(xAlarmQueue, &alarmInfoToSend,  pdMS_TO_TICKS(5000));

        if (!xResult){
            // Körs endast vid TIMEOUT
            manageBLE(&heartbeat);
        } else {
            // Körs endast vid KÖ / LARM (= pdPASS/TRUE)
            manageBLE(&alarmInfoToSend);
        }
        vTaskDelay(100);
    }
}