#include "Arduino_LED_Matrix.h"
#include "alarm.h"
#include "wifi_manager.h"
ArduinoLEDMatrix matrix;

#define LED_OFF_TIME 1000
#define LED_ON_TIME 500

bool ledActive = false;
unsigned long ledClock = 0;

typedef struct{
    uint32_t data[3];
}LedFrame;

const LedFrame LED_ON_FRAME = { 0x0, 0x600600, 0x0};
const LedFrame LED_OFF_FRAME = { 0x0, 0x0, 0x0 };


void initMatrix(){
    matrix.begin();
}

int statusLED(){
    if (!ledActive && (node.sysTime - ledClock >= LED_OFF_TIME)){
        matrix.loadFrame(LED_ON_FRAME.data);
        ledActive = true;
        ledClock = node.sysTime;
    }

    if ((ledActive && (node.sysTime - ledClock >= LED_ON_TIME) && node.connectionStatus.wifiIsActive)){
        matrix.loadFrame(LED_OFF_FRAME.data);
        ledActive = false;
        ledClock = node.sysTime;
    }

    return 0;
}