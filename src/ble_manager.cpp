#include <ArduinoBLE.h>
#include <stdbool.h>
#include "alarm.h"

// UUID https://www.uuidgenerator.net/
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");

// BLEIndicate - skickar med ACK, "leveransgaranti"
BLECharacteristic levelCharacteristic(
  "19B10001-E8F2-537E-4F6C-D104768A1214",
  BLERead | BLEIndicate,
  sizeof(AlarmInfo)
);

int sensorValue = 0;

bool initBLE(){
  if (!BLE.begin()) {
    Serial.println("BLE: Kunde inte starta BLE!");
    return false;
  }

  BLE.setLocalName("SENS_NODE");
  BLE.setAdvertisedService(customService);

  customService.addCharacteristic(levelCharacteristic);
  BLE.addService(customService);

  levelCharacteristic.writeValue((uint8_t *)&alarmInfo, sizeof(alarmInfo));  

  BLE.advertise();

  Serial.println("BLE: Annonserar och väntar på klient...");
  return true;
}


void manageBLE() {
    BLEDevice central = BLE.central();

  // När en klient ansluter
    if (central) {
      if (!node.connectionStatus.bleIsActive){
        Serial.print("BLE: Ansluten till: ");
        Serial.println(central.address()); 
        node.connectionStatus.bleIsActive = true;
      }
         
      // Kör så länge klienten är ansluten [testar med IF för att göra den non-blocking]
      if (central.connected()) {     
         

        // ersätt med: 'alarmInfo'
        levelCharacteristic.writeValue((uint8_t *)&alarmInfo, sizeof(alarmInfo));  

        Serial.print("BLE: Sent!\n");
      }  else {
        Serial.println("BLE: Klient kopplade ner");
        node.connectionStatus.bleIsActive = false;
      }
    } 
    // Viktigt: låt BLE jobba [oavsätt om vi är connectade eller ej]
    BLE.poll();
}