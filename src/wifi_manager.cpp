#include <WiFiS3.h>
#include "alarm.h"
#include "indicateStatus.h"

// kolla om vi är anslutan till wifi
// retunerar true om connected.
bool wifiIsConnected(){
    return (WiFi.status() == WL_CONNECTED);
}

// init, bara vid uppstart!
void initWiFi(){
    WiFi.begin(WIFI_SSID, WIFI_PASS);  
}

// hanterar wifi
int manageWiFi(){
    if (!wifiIsConnected()){
        if (node.connectionStatus.wifiIsActive){
            node.connectionStatus.wifiIsActive = false;
        Serial.println("\n\n..WiFi disconneced..");
        Serial.println("");
        return false;
        }

    } else {
        if (!node.connectionStatus.wifiIsActive){
            node.connectionStatus.wifiIsActive = true;
            Serial.print("\n\nConneced to WiFi: ");
            Serial.println(WIFI_SSID);
            Serial.println("");
            return true;
        }
    }
}