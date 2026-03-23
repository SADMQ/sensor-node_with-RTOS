#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "tasks.h"
#include "alarm.h"
#include "sensor_motion.h"
#include "sensor_reed.h"
#include "indicateStatus.h"

SemaphoreHandle_t xAlarmSemaphore;
SemaphoreHandle_t xSystemMonitorSemaphore;
SemaphoreHandle_t xNetworkSemaphore;
TimerHandle_t xLEDTimer;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  initComponents();
  Serial.println("--- STARTING SYSTEM ---");

  // skapa timer
  xLEDTimer = xTimerCreate("LED_STATUS", pdMS_TO_TICKS(idleLEDSpeed), pdTRUE, 0, vLEDTimerCallback);
  // starta timer
  xTimerStart(xLEDTimer, 0);

  xAlarmSemaphore = xSemaphoreCreateBinary();
  xSystemMonitorSemaphore = xSemaphoreCreateBinary();
  xNetworkSemaphore = xSemaphoreCreateBinary();

  //attachInterrupt(digitalPinToInterrupt(mq2Pin), smokeIsDetected, RISING); - Körs digitalt (DO).
  attachInterrupt(digitalPinToInterrupt(reedPin), reedIsTriggerd, RISING);
  attachInterrupt(digitalPinToInterrupt(pirPin), motionIsDetected, RISING);

  xTaskCreate(vAlarmTask, "ALARM", 192, NULL, 3, NULL); // OBS! Kan behöva ökas när vi ökar antal sensorer här.
  xTaskCreate(vSystemMonitorTask, "MONITOR", 192, NULL, 2, NULL);
  xTaskCreate(vNetworkTask, "NET", 1024, NULL, 1, NULL);
  
  vTaskStartScheduler();
}

void loop() {
}

extern "C" void vApplicationTickHook(void) {
  node.sysTime++;
}