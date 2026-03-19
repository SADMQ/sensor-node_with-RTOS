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
  //  if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
  //    node.alarmStatus.fireAlarm = true;
  //    Serial.println("\n--FIRE DETECTED--\n");
  //  } else {
  //    node.alarmStatus.fireAlarm = false;
  //  }
  //  if (node.sensors.motionDetect == true){
  //    node.alarmStatus.intrusionAlarm = true;
  //    Serial.println("\n--MOTION DETECTED--\n");
  //  }
    if (node.sensors.waterLeak == true){
      node.alarmStatus.waterLeak = true;
       Serial.println("\n--WATER-LEAK DETECTED--\n");
    }
    return 0;

  case STATE_ARMED_HOME:
    if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
      node.alarmStatus.fireAlarm = true;
      Serial.println("\n--FIRE DETECTED--\n");
    } else {
      node.alarmStatus.fireAlarm = false;
    }
    if (node.sensors.waterLeak == true){
      node.alarmStatus.waterLeak = true;
       Serial.println("\n--WATER-LEAK DETECTED--\n");
    }
    return 0;

  case STATE_DISARMED:
    if (node.sensors.smokeSensor == true || (node.sensors.fireTemp >= DS18B20_ALARMING_TEMP)){
      node.alarmStatus.fireAlarm = true;
      Serial.println("\n--FIRE DETECTED--\n");
      } else {
        node.alarmStatus.fireAlarm = false;
      }
      if (node.sensors.waterLeak == true){
        node.alarmStatus.waterLeak = true;
         Serial.println("\n--WATER-LEAK DETECTED--\n");
      }
      if (node.sensors.indoorHumidity >= 70){  // bara för test
      Serial.println("\n--HIGH HUMIDITY DETECTED--\n");  // bara för test
      }
    return 0;

  }
}