#ifndef SENSORS_H
#define SENSORS_H
#include <Arduino.h>
#include <stdio.h>

enum SensorStateP3 {
    READING_DHT,
    READING_WATER
};
enum SensorStateP2 {
    READING_DS18B20,
    READING_MQ2
};

void sensorsInit();
void readPrio3Sensors();



#endif