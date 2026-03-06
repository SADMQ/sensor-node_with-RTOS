#include <stdio.h>
#include "tasks.h"
#include <Arduino.h>
#include "alarm.h"

//definierar node-strukten (samt deklarera nässlade struktar)
System node = {
  .runStatus = WAKING_UP,
  .connectionStatus = {
    .wifiIsActive = false,
    .bleIsActive = false,
  },
  .alarmMode = STATE_DISARMED,  // ska initieras som "STATE_DISARMED".
  .alarmStatus = {
    .intrusionAlarm = false,
    .fireAlarm = false,
    .waterLeak = false,
    .systemFailure = false
  },
  .sensors = {
    .reedSensor1 = false,
    .reedSensor2 = false,
    .motionDetect = false,
    .smokeSensor = false,
    .fireTemp = 0.0,
    .indoorTemp = 0.0,
    .indoorHumidity = 0.0,
    .waterLeak = false,
  }
};


void checkAlarmStatus(){ 
  switch (node.alarmMode)
  {
  case STATE_ARMED_AWAY:
      if (node.sensors.smokeSensor == true || node.sensors.fireTemp == true){
        node.alarmStatus.fireAlarm == true;
        printf("\n--FIRE DETECTED--\n");
      }
      if (node.sensors.motionDetect == true){
        node.alarmStatus.intrusionAlarm == true;
        printf("\n--MOTION DETECTED--\n");
      }
      if (node.sensors.reedSensor1 == true || node.sensors.reedSensor2 == true){
        node.alarmStatus.intrusionAlarm == true;
        printf("\n--DOOR/WINDOW OPEND!--\n");
      }
      if (node.sensors.waterLeak == true){
        node.alarmStatus.waterLeak == true;
         printf("\n--WATER-LEAK DETECTED--\n");
      }
  break;

  case STATE_ARMED_HOME:
            if (node.sensors.smokeSensor == true || node.sensors.fireTemp == true){
              node.alarmStatus.fireAlarm == true;
        printf("\n--FIRE DETECTED--\n");
      }
      if (node.sensors.reedSensor1 == true || node.sensors.reedSensor2 == true){
        node.alarmStatus.intrusionAlarm == true;
        printf("\n--DOOR/WINDOW OPEND!--\n");
      }
      if (node.sensors.waterLeak == true){
        node.alarmStatus.waterLeak == true;
         printf("\n--WATER-LEAK DETECTED--\n");
      }
  break;

  case STATE_DISARMED:
      if (node.sensors.smokeSensor == true || node.sensors.fireTemp == true){
        node.alarmStatus.fireAlarm == true;
        printf("\n--FIRE DETECTED--\n");
      }
      if (node.sensors.waterLeak == true){
        node.alarmStatus.waterLeak == true;
         printf("\n--WATER-LEAK DETECTED--\n");
      }
      if (node.sensors.indoorHumidity >= 70){  // bara för test
      printf("\n--HIGH HUMIDITY DETECTED--\n");  // bara för test
      }
  break;

  }
}