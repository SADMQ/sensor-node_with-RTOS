#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "tasks.h"
#include "alarm.h"

SemaphoreHandle_t xAlarmSemaphore;
SemaphoreHandle_t xSystemMonitorSemaphore;
SemaphoreHandle_t xNetworkSemaphore;

void setup() {
  Serial.begin(115200);
  delay(2000); // while? behövs?
  initComponents();
  Serial.println("--- STARTING SYSTEM ---");

  //xAlarmSemaphore = xSemaphoreCreateBinary();
  xSystemMonitorSemaphore = xSemaphoreCreateBinary();
  xNetworkSemaphore = xSemaphoreCreateBinary();

  //xTaskCreate(vAlarmTask, "ALARM", 128, NULL, 3, NULL);
  xTaskCreate(vSystemMonitorTask, "MONITOR", 512, NULL, 2, NULL);
  xTaskCreate(vNetworkTask, "NETWORK", 1024, NULL, 1, NULL);

  Serial.println("Starting FreeRTOS Scheduler...");
  
  // 3. STARTA SKEDULERAREN MANUELLT (Kritiskt för R4)
  vTaskStartScheduler();

  // Om vi når hit har något gått snett
  Serial.println("Insufficient RAM to start Scheduler!");
}

void loop() {
}

extern "C" void vApplicationTickHook(void) {
  node.sysTime++;
}