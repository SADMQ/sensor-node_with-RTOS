// --- definierar NÄR resp. funktion ska köras ---
#include <stdio.h>
#include <Arduino.h>
#include "tasks.h"
#include "alarm.h"
#include "wifi_manager.h"
#include "indicateStatus.h"
#include "mqtt_client.h"
#define WAKE_UP_SYSTEM_MS 20000 // kanske 60s ? (MQ2 + PIR behöver tid..)

void taskScheduler(){
  while (true){
    
  }
};

void startingSystem(){
    if (node.runStatus == WAKING_UP){
      if ((manageWiFi()) && (node.sysTime >= WAKE_UP_SYSTEM_MS) && (manageMQTT())) {
        node.runStatus = RUNNING;
        Serial.println("\n<<< SYSTEM IS READY >>>\n\n");
      }
    }
}