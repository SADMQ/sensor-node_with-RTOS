#include "Arduino_LED_Matrix.h"
#include <Arduino_FreeRTOS.h>
#include "indicateStatus.h"
#include "alarm.h"
#include "wifi_manager.h"
ArduinoLEDMatrix matrix;

bool ledActive = false;

typedef struct{
    uint32_t data[3];
}LedFrame;

const LedFrame LED_ON_FRAME = { 0x0, 0x600600, 0x0};
const LedFrame LED_FULL_ON = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
const LedFrame LED_OFF_FRAME = { 0x0, 0x0, 0x0 };


void initMatrix(){
    matrix.begin();
}

int statusLED(bool alarming){
    // släck alla leds
    if (ledActive){
        matrix.loadFrame(LED_OFF_FRAME.data);
        ledActive = false;
        return 0;
    }
    

    if (!ledActive){
    // tänd status-leds
        if (alarming){
            matrix.loadFrame(LED_FULL_ON.data);
        } else {
            matrix.loadFrame(LED_ON_FRAME.data);
        }
        ledActive = true;
    }
    return 1;
}

void vLEDTimerCallback(TimerHandle_t xTimer){
    if (!node.connectionStatus.wifiIsActive || !node.connectionStatus.mqttIsActive || !node.connectionStatus.bleIsActive){
        xTimerChangePeriod(xLEDTimer, pdMS_TO_TICKS(200), 0);
    } else {
        xTimerChangePeriod(xLEDTimer, pdMS_TO_TICKS(idleLEDSpeed), 0);
    }
    
    if (node.alarmStatus.intrusionAlarm || node.alarmStatus.fireAlarm) {
        statusLED(1);
        return;
    }

    statusLED(0);
}