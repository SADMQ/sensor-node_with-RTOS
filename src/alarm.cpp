#include <stdio.h>
#include "tasks.h"
#include <Arduino.h>
#include <RTC.h>
#include <stdio.h>
#include "alarm.h"
#include <WiFi.h>
#include <Arduino_FreeRTOS.h>
#define DS18B20_ALARMING_TEMP 60 // Temp: ca 60

AlarmInfo alarmInfo =  {NONE, 0};

char timestamp[25];

uint32_t getUnixTime(){
  RTCTime currentTime;
  RTC.getTime(currentTime);
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
};


int checkAlarmStatus(){ 
  // Water-leak
  if (node.sensors.waterLeak == true){
    node.alarmStatus.waterLeak = true;
    
    // lagrar vad & när i struct.
    alarmInfo.time = getUnixTime();
    alarmInfo.trigger = WATER;
    // lägg i Queue!
    xQueueSend(xAlarmQueue, &alarmInfo, 0);

    Serial.print(timestamp);
    Serial.println("\n--WATER-LEAK DETECTED--\n");
  } else {
    node.alarmStatus.waterLeak = false;
  }
  // Fire
  if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
    node.alarmStatus.fireAlarm = true;

    // lagrar vad & när i struct.
    alarmInfo.time = getUnixTime();
    alarmInfo.trigger = FIRE;
    // lägg i Queue!
    xQueueSend(xAlarmQueue, &alarmInfo, 0);

    // time stamp 
    Serial.print(timestamp);
    Serial.println("\n--FIRE DETECTED--\n");
  } else {
    node.alarmStatus.fireAlarm = false;
  }


  switch (node.alarmMode)
  {
  case STATE_ARMED_AWAY:
    // Reed (door / widnow sensor)
    if (node.sensors.reedSensor1 == true){
      node.alarmStatus.intrusionAlarm = true;
      // lagrar vad & när i struct.
      alarmInfo.time = getUnixTime();
      alarmInfo.trigger = DOOR;
      // lägg i Queue!
      xQueueSend(xAlarmQueue, &alarmInfo, 0);
      
      // time stamp 
      Serial.print(timestamp);
      Serial.println("\n--DOOR/WINDOW DETECTED--\n");
    }

    // Motion
    if (node.sensors.motionDetect == true){
      node.alarmStatus.intrusionAlarm = true;

      // lagrar vad & när i struct.
      alarmInfo.time = getUnixTime();
      alarmInfo.trigger = MOTION;
      // lägg i Queue!
      xQueueSend(xAlarmQueue, &alarmInfo, 0);

      // time stamp 
      Serial.print(timestamp);
      Serial.println("\n--MOTION DETECTED--\n");
    }
    return 0;

  case STATE_ARMED_HOME:
    // Reed (door / widnow sensor)
      if (node.sensors.reedSensor1 == true){
      node.alarmStatus.intrusionAlarm = true;
      // lagrar vad & när i struct.
      alarmInfo.time = getUnixTime();
      alarmInfo.trigger = DOOR;
      // lägg i Queue!
      xQueueSend(xAlarmQueue, &alarmInfo, 0);

      Serial.print(timestamp);
      Serial.println("\n--DOOR/WINDOW DETECTED--\n");
    }
    return 0;

  case STATE_DISARMED:
    return 0;
  }
}