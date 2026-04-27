#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "tasks.h"
#include "alarm.h"
#include "sensor_motion.h"
#include "sensor_reed.h"
#include "indicateStatus.h"
#include "ble_manager.h"
#define TIMER_ENTRY_TIME 30000
#define HEARTBEAT_MISSING_TRIGGER 15000
// #define TIMER_EXIT_TIME 30000 -- flyttar till ESP..

SemaphoreHandle_t xAlarmSemaphore;
SemaphoreHandle_t xSystemMonitorSemaphore;
SemaphoreHandle_t xNetworkSemaphore;
TimerHandle_t xLEDTimer;
TimerHandle_t xAlarmEntryTimer; // Används för aktivering (tid innan larmning) samt inför triggning av larm (tid för avlarming).
TimerHandle_t xHeartbeatTimer;  // Används för att övervaka heartbeat från ESP.
QueueHandle_t xAlarmQueue;      // BLE-Send kö - för ESP.
QueueHandle_t xMessageQueue;    // MQTT-Send kö - för broker.
QueueHandle_t xHeartbeatStateQueue;    // BLE-receive kö.

extern "C" void vApplicationTickHook(void) {
  node.sysTime++;
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  initComponents();
  Serial.println("--- STARTING SYSTEM ---");

  xLEDTimer = xTimerCreate("LED_STATUS", pdMS_TO_TICKS(idleLEDSpeed), pdTRUE, 0, vLEDTimerCallback);
  xAlarmEntryTimer = xTimerCreate("ALARM_TIMER_ENTRY", pdMS_TO_TICKS(TIMER_ENTRY_TIME), pdFALSE, 0, vAlarmTimerCallback); 
  xHeartbeatTimer = xTimerCreate("HEARTBEAT_TIMER", pdMS_TO_TICKS(HEARTBEAT_MISSING_TRIGGER) , pdTRUE, 0, vHeartbeatMissingCallback);

  xTimerStart(xLEDTimer, 0);

  xAlarmSemaphore = xSemaphoreCreateBinary();
  xSystemMonitorSemaphore = xSemaphoreCreateBinary();
  xNetworkSemaphore = xSemaphoreCreateBinary();
  xAlarmQueue = xQueueCreate(10, sizeof(AlarmInfo));
  xMessageQueue = xQueueCreate(10, sizeof(AlarmInfo));
  xHeartbeatStateQueue = xQueueCreate(1, sizeof(AlarmMode));

  //attachInterrupt(digitalPinToInterrupt(mq2Pin), smokeIsDetected, RISING); - Körs digitalt (DO).
  attachInterrupt(digitalPinToInterrupt(reedPin), reedIsTriggerd, RISING);
  attachInterrupt(digitalPinToInterrupt(pirPin), motionIsDetected, RISING);

  xTaskCreate(vAlarmTask, "ALARM", 192, NULL, 4, NULL);
  xTaskCreate(vBLETask, "BLE", 512, NULL, 3, NULL);
  xTaskCreate(vNetworkTask, "WIFI", 1024, NULL, 3, NULL);
  xTaskCreate(vSystemMonitorTask, "MONITOR", 192, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {
}