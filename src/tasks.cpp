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
        RTC.begin();
        RTCTime startTime(epoch);
        RTC.setTime(startTime); // Nu är klockan ställd!
        Serial.println("RTC: Clock synchronized!");
        return true;
   }
        return false;
}

int initTimeNTP(){ 
        //RTC.begin();
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
        BaseType_t xResult = xSemaphoreTake(xAlarmSemaphore, pdMS_TO_TICKS(2000)); 

            // kör endast vid semaphore
            if (xResult){
                if (node.sensors.HWEvent_motionDetect){
                    node.sensors.motionDetect = true;
                    checkAlarmStatus();
                    node.sensors.HWEvent_motionDetect = false;  // bör inte nollas här..?
                    node.sensors.motionDetect = false;          // bör inte nollas här..?
                }

                if (node.sensors.HWEvent_reedSensor1){
                    node.sensors.reedSensor1 = true; 
                    checkAlarmStatus();
                    node.sensors.HWEvent_reedSensor1 = false;   // bör inte nollas här..?
                    node.sensors.reedSensor1 = false;           // bör inte nollas här..?
                }
            } else {
                // går ENDAST på tidsintevall - oberoende semaphore, ~2000ms. 
                getDS18B20data();
                checkAlarmStatus();
                }

         // Checka om något larm är triggat.
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


// RTOS: Task - hanterar nätverk -> WIFI, MQTT & (BLE..)
// Händelsestyrd & tidsstyrd
void vNetworkTask(void *Params){
    // körs bara EN gång
    initWiFi();
    static int iterationCntr = 0;
    AlarmInfo mqttToSend;
    Serial.println("WiFi: Init - Complete!");

    for (;;){
        
        // behöver vakna vid LARM / STATE-update (samt timeout för Wifi + MQTT..)
        BaseType_t xResult = xQueueReceive(xMessageQueue, &mqttToSend,  pdMS_TO_TICKS(1000));

        manageWiFi();
        if (wifiIsConnected()){

            // sätter tiden @ WIFI
            if (!node.timeIsSet){
                node.timeIsSet = initTimeWiFi();
            }
            // synca tiden mot broker ----- [ Inaktivera pga MQTT problem...? ]
            if (!node.NTCsynced && node.connectionStatus.mqttIsActive){
               node.NTCsynced = initTimeNTP();
            }

            if (xResult){
                // ::event::
                sendMQTT(&mqttToSend); 
            }
            
            manageMQTT(); 
        }
        vTaskDelay(3000);   
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

        alarmInfo.climate.inTemp = (int16_t)(node.sensors.indoorTemp*100.0f);
        alarmInfo.climate.inHum = (uint16_t)(node.sensors.indoorHumidity*100.0f);

        if (!xQueueSend(xAlarmQueue, &alarmInfo, pdMS_TO_TICKS(100))){
            Serial.println("! Kunde ej skicka larm till MessagesQueue !");
        }
    } else {
        Serial.print("DHT11: ERROR");
    }


    Serial.println("WL: No water leak..\n"); 
    // kolla water leak sensorn här..
}


void vBLETask(void* Params){
    initBLE();
    AlarmInfo alarmInfoToSend;
    AlarmMode newState;
    Serial.println("BLE: Init - Complete!");

    for (;;){
        BaseType_t xResult = xQueueReceive(xAlarmQueue, &alarmInfoToSend,  pdMS_TO_TICKS(100));
        
        if (xResult){   // vid KÖ / LARM (= pdPASS/TRUE)
            manageBLE(&alarmInfoToSend);   
        } else {        // vid Timeout
            manageBLE(nullptr);
        }

        while (xQueueReceive(xHeartbeatStateQueue, &newState, 0)){
            xTimerReset(xHeartbeatTimer, 0);
            Serial.print("\nBLE Rx (ESP): Heartbeat-state: ");
            Serial.println(newState); 

            if (node.alarmMode != (AlarmMode)newState) {
                handleStateChange((AlarmMode)newState);
            }
        } 
        vTaskDelay(pdMS_TO_TICKS(750)); // 750??
    }
}
