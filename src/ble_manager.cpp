#include <ArduinoBLE.h>
#include <stdbool.h>

// UUID https://www.uuidgenerator.net/
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLEIntCharacteristic levelCharacteristic(
  "19B10001-E8F2-537E-4F6C-D104768A1214",
  BLERead | BLENotify
);

int sensorValue = 0;
unsigned long lastUpdate = 0;

bool initBLE(){
  if (!BLE.begin()) {
    Serial.println("BLE: Kunde inte starta BLE!");
    return false;
  }

  BLE.setLocalName("SENS_NODE");
  BLE.setAdvertisedService(customService);

  customService.addCharacteristic(levelCharacteristic);
  BLE.addService(customService);

  levelCharacteristic.writeValue(sensorValue);

  BLE.advertise();

  Serial.println("BLE: Arduino annonserar och väntar på klient...");
  return true;
}


void manageBLE() {
    static bool bleClientConnected = false;
    BLEDevice central = BLE.central();

  // När en klient ansluter
    if (central) {
      if (!bleClientConnected){
        Serial.print("BLE: Ansluten till: ");
        Serial.println(central.address()); 
        bleClientConnected = true;
      }
         
      // Kör så länge klienten är ansluten [testar med IF för att göra den non-blocking]
      if (central.connected()) {    
        lastUpdate = millis();    
        sensorValue++;    
        levelCharacteristic.writeValue(sensorValue); // notify    
        Serial.print("BLE: Skickar: ");
        Serial.println(sensorValue);
      }  else {
        Serial.println("BLE: Klient kopplade ner");
        bleClientConnected = false;
      }
    } 
    // Viktigt: låt BLE jobba [oavsätt om vi är connectade eller ej]
    BLE.poll();
}