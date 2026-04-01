#include <Arduino.h>
#include "Arduino_FreeRTOS.h"
#include "alarm.h"

void initReed(){
    // NOTERA: koppla D3 --> REED --> GND (alt. bara en kabel, när den bryts - triggas larmet)
    pinMode(reedPin, INPUT_PULLUP);
}

// indikera ATT vi triggat
void reedIsTriggerd(){
    // Initierar variabel (för prio-besked till RTOS)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    node.sensors.HWEvent_reedSensor1 = true;

    // Flagga semaforen och lagrar prio-svaret.  Om påkallad Task (Alarm) har Högre prio -> 'HigherPriorityTaskWoken' blir pdTRUE.
    xSemaphoreGiveFromISR(xAlarmSemaphore, &xHigherPriorityTaskWoken);
    
    // Tvinga RTOS byta task omedelbart, om prio är högre.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
