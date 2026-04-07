#include <ArduinoBLE.h>
#include <stdbool.h>
#include "alarm.h"

// UUID https://www.uuidgenerator.net/
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");

// Characteristic för att TA EMOT larmstatus (1 byte)
BLEByteCharacteristic stateCharacteristic(
  "19B10002-E8F2-537E-4F6C-D104768A1214", 
  BLERead | BLEWrite
);

// Characteristic för att SKICKA larm-info (5 byte)
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
  customService.addCharacteristic(stateCharacteristic);

  BLE.addService(customService);

  stateCharacteristic.writeValue((uint8_t)node.alarmMode); 
  levelCharacteristic.writeValue((uint8_t *)&alarmInfo, sizeof(alarmInfo));  

  BLE.advertise();

  Serial.println("BLE: Annonserar och väntar på klient...");
  return true;
}


void manageBLE(const AlarmInfo *alarmData) {
  // 1. Låt BLE uppdatera sin interna status
    BLE.poll();

    // 2. Hämta den anslutna klienten
    BLEDevice central = BLE.central();

  // När en klient ansluter
    if (central) {
      // Om detta är en ny anslutning
      if (!node.connectionStatus.bleIsActive){
        Serial.print("BLE: Ansluten till: ");
        Serial.println(central.address()); 
        node.connectionStatus.bleIsActive = true;
      }
         
      // Kör så länge klienten är ansluten
      if (central.connected()) {     
  
        // skicka larmdata, via BLE - alt heartbeat 'NULL'
        levelCharacteristic.writeValue((uint8_t *)alarmData, sizeof(AlarmInfo)); 

        // DEBUG (visar larmstrukten som skickas till ESP..)
        Serial.print("\n\n------BLE_DATA------");
        Serial.print("\nBLE: Skickar till: ");
        Serial.print(central.address());
        Serial.print("\nBLE: data sent: ");
        Serial.print(alarmData->trigger);
        Serial.print("\nBLE: time sent: ");
        Serial.print(alarmData->time);
        Serial.print("\n------BLE_DATA------\n\n");
        
        // Ta emot data från ESP32 - "alarm-state"
        if (stateCharacteristic.written()){
          uint8_t receivedState = stateCharacteristic.value();

          // uppdatera internt state med mottaget state
          node.alarmMode = (AlarmMode)receivedState;
          Serial.print("\nBLE: Nytt state mottaget: ");
          Serial.println(node.alarmMode);
        }
      }  else {
        Serial.println("BLE: Klient kopplade ner");
        node.connectionStatus.bleIsActive = false;
      }
    } 
}