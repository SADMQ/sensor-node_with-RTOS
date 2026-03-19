#include "sensor_motion.h"
#include <Arduino.h>
#include "alarm.h"



void initPIR(){
    pinMode(pirPin, INPUT);
}


void motionIsDetected(){
    // Initierar variabel (för prio-besked ifrån RTOS)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    node.sensors.HWEvent_motionDetect = true;

    // Flagga semaforen och lagrar prio-svaret. 
    xSemaphoreGiveFromISR(xAlarmSemaphore, &xHigherPriorityTaskWoken);

    // Tvinga RTOS byta task omedelbart, om prio är högre.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


    //if (node.sysTime > 70000 && node.alarmMode == STATE_ARMED_AWAY){ // warm-up time
    //node.sensors.motionDetect = true;
    // add trigger-time ?
    //}
