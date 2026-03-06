#include <WiFiS3.h>
#include "alarm.h"

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
void manageWiFi(){
    if (!wifiIsConnected()){
        if (node.connectionStatus.wifiIsActive){
            node.connectionStatus.wifiIsActive = false;
        printf("\n\n..WiFi disconneced..");
        printf("\n\n");
        }

    } else {
        if (!node.connectionStatus.wifiIsActive){
            node.connectionStatus.wifiIsActive = true;
            printf("\n\nConneced to WiFi: ");
            printf(WIFI_SSID);
            printf("\n\n");
        }
    }
}