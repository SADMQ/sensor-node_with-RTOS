// --- definierar HUR (sekvensen) och VAD som ska köras --- 
#include "tasks.h"
#include "sensor_dht11.h"
#include <stdio.h>
#include <Arduino.h>
#define numOfPrio3Sensors 2

enum SensorState {
    READING_DHT,
    READING_WATER
};

SensorData currentStatus = {
    .reedSensor1 = false,
    .reedSensor2 = false,
    .motionDetect = false,
    .smokeSensor = false,
    .fireTemp = 0.0,
    .indoorTemp = 0.0,
    .indoorHumidity = 0.0,
    .waterLeak = false
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
        Serial.print(currentStatus.indoorTemp, 1); // 1 decimal
        Serial.print(" C | Fukt: ");
        Serial.print(currentStatus.indoorHumidity, 1);
        Serial.println(" %");
        // -- DEBUG --

        currentSensor = READING_WATER; 
        break;

        
    case READING_WATER:
        //läs water leak
        currentSensor = READING_DHT;
        break;
    }
};