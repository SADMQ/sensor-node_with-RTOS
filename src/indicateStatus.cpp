#include "Arduino_LED_Matrix.h"
#include "alarm.h"
ArduinoLEDMatrix matrix;

#define LED_OFF_TIME 1500
#define LED_ON_TIME 500
#define MATRIX_LED 54

bool ledActive = false;
unsigned long ledClock = 0;


const uint32_t LED_ON_FRAME[] = {
    0x0, 
    0x600600,   // tänder pixlar i centrum
    0x0
};

const uint32_t LED_OFF_FRAME[] = {
    0x0, 0x0, 0x0 // släcker alla pixlar..
};


void initMatrix(){
    matrix.begin();
}

void statusLED(){
    if (!ledActive && (node.sysTime - ledClock >= LED_OFF_TIME)){
        matrix.loadFrame(LED_ON_FRAME); //matrix.on(MATRIX_LED);
        ledActive = true;
        ledClock = node.sysTime;
    }

    if (ledActive && (node.sysTime - ledClock >= LED_ON_TIME)){
        matrix.loadFrame(LED_OFF_FRAME); //matrix.off(MATRIX_LED);
        ledActive = false;
        ledClock = node.sysTime;
    }
    
}