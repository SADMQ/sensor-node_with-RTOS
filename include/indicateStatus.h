#ifndef INDICATE_STATUS_H
#define INDICATE_STATUS_H
#define idleLEDSpeed 1000

void initMatrix();
int statusLED(bool alarming);

extern TimerHandle_t xLEDTimer; 
void vLEDTimerCallback(TimerHandle_t xTimer);

#endif