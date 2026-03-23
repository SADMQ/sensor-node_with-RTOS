#ifndef INDICATE_STATUS_H
#define INDICATE_STATUS_H

void initMatrix();
int statusLED(bool alarming);

extern TimerHandle_t xLEDTimer; 
void vLEDTimerCallback(TimerHandle_t xTimer);

#endif