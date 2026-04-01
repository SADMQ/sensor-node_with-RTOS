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
QueueHandle_t xAlarmQueue;

extern "C" void vApplicationTickHook(void) {
  node.sysTime++;
}

extern "C" void vApplicationTickHook(void) {
  node.sysTime++;
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  initComponents();
  Serial.println("--- STARTING SYSTEM ---");

  xLEDTimer = xTimerCreate("LED_STATUS", pdMS_TO_TICKS(idleLEDSpeed), pdTRUE, 0, vLEDTimerCallback);
  xTimerStart(xLEDTimer, 0);

  xAlarmSemaphore = xSemaphoreCreateBinary();
  xSystemMonitorSemaphore = xSemaphoreCreateBinary();
  xNetworkSemaphore = xSemaphoreCreateBinary();
  xAlarmQueue = xQueueCreate(10, sizeof(AlarmInfo));

  //attachInterrupt(digitalPinToInterrupt(mq2Pin), smokeIsDetected, RISING); - Körs digitalt (DO).
  attachInterrupt(digitalPinToInterrupt(reedPin), reedIsTriggerd, RISING);
  attachInterrupt(digitalPinToInterrupt(pirPin), motionIsDetected, RISING);

  xTaskCreate(vAlarmTask, "ALARM", 192, NULL, 4, NULL); // OBS! Kan behöva ökas när vi ökar antal sensorer här.
  xTaskCreate(vNetworkTask, "NET", 1024, NULL, 3, NULL);
  xTaskCreate(vSystemMonitorTask, "MONITOR", 192, NULL, 1, NULL);
  
  vTaskStartScheduler();
}

void loop() {
}