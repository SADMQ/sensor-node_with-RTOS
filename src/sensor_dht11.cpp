#include "tasks.h"
#include "config.h"
#include "sensor_dht11.h"
#include "alarm.h"

DHT dht(DHT_PIN, DHT_TYPE, 6);

void initDHT() {
    dht.begin(); // startsekvensen
    Serial.println("DHT Initierad");
    // node.dhtReady = true;
}

bool getDHTData() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        node.sensors.indoorTemp = -127.0;       // error value
        node.sensors.indoorHumidity = -127.0;   // error value
        return false; 
    }

    node.sensors.indoorTemp = t;
    node.sensors.indoorHumidity = h;
    return true;
}
