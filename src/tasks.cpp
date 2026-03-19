// --- definierar HUR (sekvensen) och VAD som ska köras --- 
#include "tasks.h"
#include "alarm.h"
#include "sensor_dht11.h"
#include <stdio.h>
#include <Arduino.h>
#include "sensor_ds18b20.h"
#include "indicateStatus.h"
#include "wifi_manager.h"
#include "mqtt_client.h"
#include "sensor_motion.h"
#include "sensor_reed.h"
#include "scheduler.h"
#define LOW_PRIO_SENSORS_READ 5000 // 30s (TEST: 5s)

void initComponents(){
    initDHT();
    //initDS18B20();
    //initPIR();
    initReed();
    initMatrix();
}


// RTOS: Task - hanterar LARM, inbrott+brand (PIR+REED + GAS+TEMP)
// Händelsestyrd
void vAlarmTask(void *Params){
    // allt här körs EN gång
    for (;;){
        // väntar på given semafor - dvs. ett HW-interrupt, ELLER timeout 
        BaseType_t xResult = xSemaphoreTake(xAlarmSemaphore, portMAX_DELAY); // timeout för checka brandlarm.. 2s? Verkar sänka mqtt..

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
                xSemaphoreGive(xNetworkSemaphore);
                
            } else {
                // går ENDAST på tidsintevall - oberoende semaphore, ~2000ms. --- OBS, avstängt nu.
                getDS18B20data();
                checkAlarmStatus();
                if (node.alarmStatus.fireAlarm){
                    xSemaphoreGive(xNetworkSemaphore);
                }
            }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // pausa inte denna.
    }
}   



// RTOS: Task - hanterar nätverk -> WIFI, MQTT & (BLE..)
// Händelsestyrd & tidsstyrd
void vNetworkTask(void *Params){
    // körs bara EN gång
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
        }            
    }
}

// RTOS: Task - hanterar låg prio sensorer & status LED
// Tidsstyrd
void vSystemMonitorTask(void *Params){
    // Allt här körs EN gång
    uint32_t lastRead_LowPrioSensors = 0;

    for (;;){
        statusLED();

        if (node.sysTime - lastRead_LowPrioSensors >= LOW_PRIO_SENSORS_READ){
            readLowPrioSensors();
            lastRead_LowPrioSensors = node.sysTime;

            if (node.sensors.waterLeak == true){
                checkAlarmStatus();
            }
            xSemaphoreGive(xNetworkSemaphore);
        }
        // pausa tasken i 100ms för ge space för andra tasks.
        vTaskDelay(pdMS_TO_TICKS(100)); // pausa task, 100ms
    }
    
}


// --- bör flyttas till RTOS "ALARM"-task --- å tas bort här
int readPrio2Sensors(){
    static int currentSensor = READING_DS18B20; // static -> sätts endast EN gång (init)
    switch (currentSensor)
    {
    case READING_DS18B20: 
        getDS18B20data();
        currentSensor = READING_MQ2; 
        return 0;

        
    case READING_MQ2:
        Serial.println("Checking 'Smoke-sensor'..\n"); 
        //läs smoke sensor
        currentSensor = READING_DS18B20; 
        return 0;
    }
};


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