#include <OneWire.h>
#include <Arduino.h>
#include <DallasTemperature.h>
#include "alarm.h"

#define DS18B20_READ_FREQUENCY_MS 750

unsigned long DS18B20_ReqiuestTime;
bool DS18B20_waitingForAnsware = false;

// Definiera pin - (DI:5)
const int ONE_WIRE_BUS = 5;

// Skapa "one wire" instans
OneWire oneWire(ONE_WIRE_BUS);

// Skapa "sensorn"
DallasTemperature ds18b20(&oneWire);

void initDS18B20() {
    ds18b20.begin(); // startsekvensen
    // AVAKTIVERA väntetiden (gör den non-blocking)
    ds18b20.setWaitForConversion(false); 
    Serial.println("DS18B20: Initierad");
}

void getDS18B20data(){
    if (!DS18B20_waitingForAnsware){    // om inte frågat om temp ännu, gör det..
        ds18b20.requestTemperatures();
        DS18B20_ReqiuestTime = node.sysTime;   // räkna tiden från när vi frågar
        DS18B20_waitingForAnsware = true;

        } else {
        if (node.sysTime - DS18B20_ReqiuestTime >= DS18B20_READ_FREQUENCY_MS){ // har tillräckligt lång tid gått?
            if (ds18b20.getTempCByIndex(0) == -127){
                Serial.println("DS18B20: No data..");
            } else {
                // lagrar temperatur från sensor: ID "0" på bussen
                node.sensors.fireTemp = ds18b20.getTempCByIndex(0);
                Serial.print("DS18B20: ");
                Serial.print(node.sensors.fireTemp, 1); 
                Serial.println(" °C");
            }

            DS18B20_waitingForAnsware = false; // nu väntar vi inte längre.. sätt till false

        }
    }
}