#include <stdio.h>
#include "tasks.h"
#include <Arduino.h>
#include <RTC.h>
#include <stdio.h>
#include "alarm.h"
#include <WiFi.h>
#include <Arduino_FreeRTOS.h>
#define DS18B20_ALARMING_TEMP 60 // Temp: ca 60

AlarmInfo alarmInfo =  {STATE_DISARMED, NONE, 0, -1, {-127,-127,-1,-1},};
uint32_t lastFireTimer;
uint32_t lastWaterLeakTimer;

char timestamp[25];

uint32_t getUnixTime() {
  RTCTime currentTime;
  if (!RTC.getTime(currentTime)) {
      return 0; // Returnera 0 istället för skräp om läsningen misslyckas
  }
  return (uint32_t)currentTime.getUnixTime();
}

//definierar node-strukten (samt deklarera nässlade struktar)
System node = {
  .runStatus = WAKING_UP,
  .connectionStatus = {
    .wifiIsActive = false,
    .bleIsActive = false,
    .mqttIsActive = false,
  },
  .alarmMode = STATE_DISARMED,  // <<------ BARA FÖR TEST ------ >> ska initieras som "STATE_DISARMED".
  .alarmStatus = {
    .intrusionAlarm = false,
    .fireAlarm = false,
    .waterLeak = false,
    .systemFailure = false
  },
  .sensors = {
    .reedSensor1 = false,
    .HWEvent_reedSensor1 = false,
    .motionDetect = false,
    .HWEvent_motionDetect = false,
    .smokeSensor = false,
    .fireTemp = 0.0,
    .indoorTemp = 0.0,
    .indoorHumidity = 0.0,
    .waterLeak = false,
  },
  .sysTime = 0,
  .timeIsSet = false,
  .NTCsynced = false,
};


int checkAlarmStatus(){ 
  // WATER-LEAK - skicka bara denna via MQTT ?
  if ((node.sysTime - lastWaterLeakTimer >= 30000) && node.sensors.waterLeak == true){
    node.alarmStatus.waterLeak = true;
    lastWaterLeakTimer = node.sysTime;
    
    alarmInfo.trigger = WATER;
    dispatchAlarm();

    Serial.println("\n--WATER-LEAK DETECTED--\n");
  } else {
    node.alarmStatus.waterLeak = false;
  }


  // FIRE
  if ((node.sysTime - lastFireTimer >= 5000) && (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP))){
    node.alarmStatus.fireAlarm = true;
    lastFireTimer = node.sysTime;

    // lagrar vad & när i struct.
    alarmInfo.trigger = FIRE;
    dispatchAlarm();

    Serial.println("\n--FIRE DETECTED--\n");
  } else {
    node.alarmStatus.fireAlarm = false;
  }

  // if No intrusion alarm is triggerd,
  if (!node.alarmStatus.intrusionAlarm){

    switch (node.alarmMode)
    {
    case STATE_ARMED_AWAY:
      // Reed (door / widnow sensor)
      if (node.sensors.reedSensor1){

        // flyttas efter timern..?
        node.alarmStatus.intrusionAlarm = true;

        // lagrar vad & när i struct. (skicka omedelbart via BLE)
        alarmInfo.trigger = DOOR;
        dispatchAlarm();
        
        // Skickar till MQTT efter 30s
        // countdown innan skarpt larm.. ESP + Arduino. 30s? 
        // starta software-timer här .. BARA om den inte redan är startad.
        if (!xTimerIsTimerActive(xAlarmEntryTimer)){
          xTimerStart(xAlarmEntryTimer, 0);
          Serial.println("\n--DOOR/WINDOW opend. Counting down.. --\n");
        }

      } 

      // Motion
      if (node.sensors.motionDetect){
        node.alarmStatus.intrusionAlarm = true;

        alarmInfo.trigger = MOTION;
        dispatchAlarm();

        Serial.println("\n--MOTION DETECTED--\n");
      } 
      return 0;

    case STATE_ARMED_HOME:
      // Reed (door / widnow sensor)
      if (node.sensors.reedSensor1){
        node.alarmStatus.intrusionAlarm = true;

        alarmInfo.trigger = DOOR;
        dispatchAlarm();

        // Skickar till MQTT efter 30s
        // countdown innan skarpt larm.. ESP + Arduino. 30s? 
        // starta software-timer här .. BARA om den inte redan är startad.
        if (!xTimerIsTimerActive(xAlarmEntryTimer)){
          xTimerStart(xAlarmEntryTimer, 0);
          Serial.println("\n--DOOR/WINDOW opend. Counting down.. --\n");
        }
      } 
      return 0;

    case STATE_DISARMED:
      return 0;
    } 
  }
}

// packa larmet med tidstämpel och skickar iväg till kö.
void dispatchAlarm(bool sharpDoorAlarm){


  // sätt tidsstämplen för larmet
  if (node.timeIsSet){
    alarmInfo.time = getUnixTime();
  }

  // skicka larmpaket till kö (BLE)
  if (!xQueueSend(xAlarmQueue, &alarmInfo, pdMS_TO_TICKS(100))){
    Serial.println("! Kunde ej skicka larm till AlarmQueue (BLE) !");
    // spara larmet i EEPROM/NVS (Flash) ?
  }
  
  if (alarmInfo.trigger != DOOR || sharpDoorAlarm) {
    // skicka larmpaket till kö (MQTT)
    if (!xQueueSend(xMessageQueue, &alarmInfo, pdMS_TO_TICKS(100))){
      Serial.println("! Kunde ej skicka larm till MessagesQueue (MQTT) !");
      // spara larmet i EEPROM/NVS (Flash) ?
    }
  }
  
  // nolla larmet
  alarmInfo.trigger = NONE;
  alarmInfo.time = 0;
}

void vAlarmTimerCallback(TimerHandle_t xTimer){
  int timerID = (int)pvTimerGetTimerID(xTimer);

  if (timerID == ALARM_ENTRY_TIMER_ID){ // NÄR STATE GÅR TILL DISAMED - GLÖM EJ NOLLA & STOPPA TIMERN -> xTimerStop()
    // här larmar vi på skarpt! (öppnat dörr, men ej larmat av i tid)
    alarmInfo.trigger = DOOR;
    dispatchAlarm(true);
    Serial.println("\n-- DOOR/WINDOW opend - ALARMING! --\n");
  }
}