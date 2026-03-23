// --- definierar HUR (sekvensen) och VAD som ska köras --- 
#include <stdio.h>
#include <Arduino.h>
#include <RTC.h>
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
#define LOW_PRIO_SENSORS_READ 2000 //  (TEST: 2s)

void initComponents(){
    RTC.begin();
    initBLE();
    initDHT();
    initDS18B20();
    //initPIR();
    initReed();
    initMatrix();
}

int initTime(){
  unsigned long epoch = WiFi.getTime(); 
  
  if (epoch != 0) {
    RTCTime startTime(epoch);
    RTC.setTime(startTime); // Nu är klockan ställd!
    Serial.println("RTC: Clock synchronized!");
    return true;
  }
  return false;
}

// RTOS: Task - hanterar LARM, inbrott+brand (PIR+REED + GAS+TEMP)
// Händelsestyrd
void vAlarmTask(void *Params){
    // allt här körs EN gång
    for (;;){
        // väntar på given semafor - dvs. ett HW-interrupt, 
        BaseType_t xResult = xSemaphoreTake(xAlarmSemaphore, pdMS_TO_TICKS(2000)); // portMAX_DELAY - DS18B20 här, verkar sänka mqtt.. one-wire?

            if (xResult == pdPASS){
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
                //xSemaphoreGive(xNetworkSemaphore);  // Aktiveras först för BLE, alt. MQTT för brandlarmet.
            } else {
                // går ENDAST på tidsintevall - oberoende semaphore, ~2000ms. 
                getDS18B20data(); // TESTAR
                }

        checkAlarmStatus(); // Checka om något larm är triggat.
        vTaskDelay(pdMS_TO_TICKS(20)); // // TESTAR
    }
}


// RTOS: Task - hanterar nätverk -> WIFI, MQTT & (BLE..)
// Händelsestyrd & tidsstyrd
void vNetworkTask(void *Params){
    // körs bara EN gång
    bool timeIsSet = false;
    initWiFi();
    Serial.println("WiFi: Init - Complete!");
    initBLE();

    for (;;){
        // väntar här - vaknar av semaphore ELLER timeout
        BaseType_t xResult = xSemaphoreTake(xNetworkSemaphore, pdMS_TO_TICKS(5000));
        
        // körs ALLTID när loopen vaknar;
        manageWiFi();
        manageBLE(); // TESTAR .. 
        if (wifiIsConnected()){
            manageMQTT();
            if (!timeIsSet){
                if (initTime()){
                timeIsSet = true;
                }
            }
        }            
    vTaskDelay(100);
    }
}

// RTOS: Task - hanterar låg prio sensorer & status LED
// Tidsstyrd
void vSystemMonitorTask(void *Params){
    // Allt här körs EN gång

    for (;;){
        readLowPrioSensors();
        checkAlarmStatus();            
        xSemaphoreGive(xNetworkSemaphore);

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