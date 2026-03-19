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


// RTOS: Task - hanterar inbrottslarm (PIR + REED)
// Händelsestyrd
void vAlarmTask(void *Params){
    // allt här körs EN gång
    for (;;){
        // väntar på given semafor - dvs. ett HW-interrupt
        if (xSemaphoreTake(xAlarmSemaphore, portMAX_DELAY) == pdPASS){

            if (node.alarmMode == STATE_ARMED_AWAY && node.sensors.HWEvent_motionDetect){
                node.sensors.motionDetect = true;
                // add trigger-time ?
                xSemaphoreGive(xNetworkSemaphore);
            }

            if ((node.alarmMode == STATE_ARMED_AWAY || node.alarmMode == STATE_ARMED_HOME) && node.sensors.HWEvent_reedSensor1){
                node.sensors.reedSensor1 = true;
                // add trigger-time ?
                xSemaphoreGive(xNetworkSemaphore);
            }

            node.sensors.HWEvent_reedSensor1 = false;
            node.sensors.HWEvent_motionDetect = false;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // pausa task, 20ms
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
        vTaskDelay(pdMS_TO_TICKS(500)); // pausa task, 500ms
}

// RTOS: Task - hanterar låg prio sensorer & status LED
// Tidsstyrd
void vSystemMonitorTask(void *Params){
    // Allt här körs EN gång
    Serial.println("Monitor Task Started!");
    uint32_t lastRead_LowPrioSensors = 0;

    for (;;){
        statusLED();

        if (node.sysTime - lastRead_LowPrioSensors >= LOW_PRIO_SENSORS_READ){
            readLowPrioSensors();
            lastRead_LowPrioSensors = node.sysTime;
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