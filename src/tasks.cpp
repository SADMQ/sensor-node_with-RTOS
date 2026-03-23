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
//#include "scheduler.h"
#define LOW_PRIO_SENSORS_READ 2000 //  (TEST: 2s)

void initComponents(){
    RTC.begin();
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
    Serial.println("Clock synchronized!");
    return true;
  }
  return false;
}

// RTOS: Task - hanterar LARM, inbrott+brand (PIR+REED + GAS+TEMP)
// Händelsestyrd
void vAlarmTask(void *Params){
    // allt här körs EN gång
    for (;;){
        // väntar på given semafor - dvs. ett HW-interrupt, ELLER timeout 
        BaseType_t xResult = xSemaphoreTake(xAlarmSemaphore, portMAX_DELAY); // DS18B20 här, verkar sänka mqtt.. one-wire?

            if (xResult == pdPASS){
                if (node.sensors.HWEvent_motionDetect){
                node.sensors.motionDetect = true;
                // add trigger-time ?
                // BLE eller MQTT? - MQTT för lagring?
                node.sensors.HWEvent_motionDetect = false;
                }

                if (node.sensors.HWEvent_reedSensor1){
                node.sensors.reedSensor1 = true;
                // add trigger-time ?
                // BLE eller MQTT? - MQTT för lagring?
                node.sensors.HWEvent_reedSensor1 = false;
                }
                checkAlarmStatus();
                //xSemaphoreGive(xNetworkSemaphore); - Aktiveras först när vi har BLE..
            } else {
                // går ENDAST på tidsintevall - oberoende semaphore, ~2000ms. --- OBS, avstängt nu.
                }
            }
        vTaskDelay(pdMS_TO_TICKS(20)); // pausa inte denna.
}


// RTOS: Task - hanterar nätverk -> WIFI, MQTT & (BLE..)
// Händelsestyrd & tidsstyrd
void vNetworkTask(void *Params){
    // körs bara EN gång
    bool timeIsSet = false;
    Serial.println("Wifi Init..");
    initWiFi();
    Serial.println("Wifi Init: Complete!");

    for (;;){

        // väntar här - vaknar av semaphore ELLER timeout
        BaseType_t xResult = xSemaphoreTake(xNetworkSemaphore, pdMS_TO_TICKS(5000));

        // körs ALLTID när loopen vaknar;
        manageWiFi();
        //manageBLE();
        if (wifiIsConnected()){
            manageMQTT();
            if (!timeIsSet){
                if (initTime()){
                timeIsSet = true;
                }
            }
        }            
    }
}

// RTOS: Task - hanterar låg prio sensorer & status LED
// Tidsstyrd
void vSystemMonitorTask(void *Params){
    // Allt här körs EN gång

    for (;;){
        readLowPrioSensors();
        getDS18B20data();   // För brandlarm - bör ev. flyttas till Alarm Task med högre prio? (men stör MQTT-connection om de körs där nu pg.a one-wire??)
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
        Serial.print("<< DHT11 ERROR >>");
    }


    Serial.println("Checking 'Water-Leak'..\n"); 
    // kolla water leak sensorn här..
}