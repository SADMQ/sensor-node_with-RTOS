#include <stdio.h>
#include "tasks.h"
#include <Arduino.h>
#include "alarm.h"
#define DS18B20_ALARMING_TEMP 60 // Temp: ca 60

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

  switch (node.alarmMode)
  {
  case STATE_ARMED_AWAY:
    // Reed (door / widnow sensor)
    if (node.sensors.reedSensor1 == true){
      node.alarmStatus.intrusionAlarm = true;
      Serial.println("\n--DOOR/WINDOW DETECTED--\n");
    }

    // Motion
    if (node.sensors.motionDetect == true){
      node.alarmStatus.intrusionAlarm = true;
      Serial.println("\n--MOTION DETECTED--\n");
    }

    // Water-leak
    if (node.sensors.waterLeak == true){
      node.alarmStatus.waterLeak = true;
      Serial.println("\n--WATER-LEAK DETECTED--\n");
    } else {
      node.alarmStatus.waterLeak = false;
    }

    // Fire
    if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
      node.alarmStatus.fireAlarm = true;
      Serial.println("\n--FIRE DETECTED--\n");
    } else {
      node.alarmStatus.fireAlarm = false;
    }
    
    return 0;

  case STATE_ARMED_HOME:
    // Reed (door / widnow sensor)
        if (node.sensors.reedSensor1 == true){
      node.alarmStatus.intrusionAlarm = true;
      Serial.println("\n--DOOR/WINDOW DETECTED--\n");
    }

    // Water-leak
    if (node.sensors.waterLeak == true){
      node.alarmStatus.waterLeak = true;
      Serial.println("\n--WATER-LEAK DETECTED--\n");
    } else {
      node.alarmStatus.waterLeak = false;
    }

    // Fire
    if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
      node.alarmStatus.fireAlarm = true;
      Serial.println("\n--FIRE DETECTED--\n");
    } else {
      node.alarmStatus.fireAlarm = false;
    }
    return 0;

  case STATE_DISARMED:
    
    // Water-leak
    if (node.sensors.waterLeak == true){
      node.alarmStatus.waterLeak = true;
      Serial.println("\n--WATER-LEAK DETECTED--\n");
    } else {
      node.alarmStatus.waterLeak = false;
    }

    // Fire
    if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
      node.alarmStatus.fireAlarm = true;
      Serial.println("\n--FIRE DETECTED--\n");
    } else {
      node.alarmStatus.fireAlarm = false;
    }
    return 0;
  }
}