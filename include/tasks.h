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
enum SensorStateP1 {
    READING_PIR,
    READING_REED1,
    READING_REED2,
    INDICATE_STATUS,
};

void initComponents();
int readLowPrioSensors();
int readPrio2Sensors();
int readPrio1Sensors();
int initTime();

#endif