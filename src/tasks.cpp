// --- definierar HUR (sekvensen) och VAD som ska köras --- 
#include "tasks.h"
#include "alarm.h"
#include "sensor_dht11.h"
#include <stdio.h>
#include <Arduino.h>
#include "sensor_ds18b20.h"
#define numOfPrio3Sensors 2

void sensorsInit(){
    initDHT();
    // initDS18B20();
}



void readPrio2Sensors(){
    static int currentSensor = READING_DS18B20; // static -> sätts endast EN gång (init)
    // för att minimiera jitter för "låg-prio" sensorer - läs asynkront, en sensor åt gången.
    switch (currentSensor)
    {
    case READING_DS18B20: 
        getDS18B20data();
        currentSensor = READING_MQ2; 
        break;

        
    case READING_MQ2:
        //läs smoke sensor
        currentSensor = READING_DS18B20; 
        break;
    }
};

void readPrio3Sensors(){
    static int currentSensor = READING_DHT; // static -> sätts endast EN gång (init)
    // för att minimiera jitter för "låg-prio" sensorer - läs asynkront, en sensor åt gången.
    switch (currentSensor)
    {
    case READING_DHT: 
        getDHTData();

        // -- DEBUG --
        Serial.print("Temp: ");
        Serial.print(node.sensors.indoorTemp, 1); // 1 decimal
        Serial.print(" C | Fukt: ");
        Serial.print(node.sensors.indoorHumidity, 1);
        Serial.println(" %");
        // -- DEBUG --

        currentSensor = READING_WATER; 
        break;

        
    case READING_WATER:
        //läs water leak
        Serial.println("Checking 'Water-Leak'..\n");
        currentSensor = READING_DHT;
        break;
    }
};