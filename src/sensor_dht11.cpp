#include "tasks.h"
#include "config.h"
#include "sensor_dht11.h"

DHT dht(DHT_PIN, DHT_TYPE, 6);

void initDHT() {
    dht.begin(); // startsekvensen
    Serial.println("DHT Initierad");
}

bool getDHTData() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        return false; 
    }

    currentStatus.indoorTemp = t;
    currentStatus.indoorHumidity = h;
    return true;
}
