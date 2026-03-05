#include <OneWire.h>
#include <Arduino.h>
#include <DallasTemperature.h>
#include "alarm.h"

// Definiera pin - (DI:4)
const int ONE_WIRE_BUS = 4;

// Skapa "one wire" instans
OneWire oneWire(ONE_WIRE_BUS);

// Skapa "sensorn"
DallasTemperature ds18b20(&oneWire);

void initDS18B20() {
    ds18b20.begin(); // startsekvensen
    // AVAKTIVERA väntetiden (gör den non-blocking)
    ds18b20.setWaitForConversion(false); 
    Serial.println("DS18B20 Initierad");
}

void getDS18B20data(){
    ds18b20.requestTemperatures();

    if (ds18b20.getTempCByIndex(0) == -127){
        //printf("DS18B20: No data..");
    } else {
        // lagrar temperatur från sensor "0" på bussen
        node.sensors.fireTemp = ds18b20.getTempCByIndex(0);
        printf("%.1f",node.sensors.fireTemp);
    }

}