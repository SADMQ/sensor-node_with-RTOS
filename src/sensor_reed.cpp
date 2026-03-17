#include <Arduino.h>
#include "alarm.h"

void initReed(){
    // NOTERA: koppla D3 --> REED --> GND (alt. bara en kabel, när den bryts - triggas larmet)
    pinMode(reedPin, INPUT_PULLUP);
}

void reedIsTriggerd(){
    if (node.alarmMode != STATE_DISARMED){
        node.sensors.reedSensor1 = true;
        // add trigger-time ?
    }
}