#ifndef ALARM_H
#define ALARM_H
#include <Arduino_FreeRTOS.h> // testa om det behövs, annars ersätt med Arduino_FreeRTOS.h

// RTOS semaphore för larm
extern SemaphoreHandle_t xAlarmSemaphore;
extern SemaphoreHandle_t xNetworkSemaphore;
extern SemaphoreHandle_t xSystemMonitorSemaphore;
extern QueueHandle_t xAlarmQueue;

// HIGH PRIO pins to monitor (HW interrupt)
const int reedPin = 3;
const int pirPin = 4;

int checkAlarmStatus();
void vAlarmTask(void *Params);
void vNetworkTask(void *Params);
void vSystemMonitorTask(void *Params);
void updCurrentTime(char* timestamp, size_t size);
void dispatchAlarm();

typedef enum
{
    WAKING_UP,
    RUNNING
}RunStatus;


typedef enum : uint8_t
{
    NONE = 0,
    WATER = 1,
    DOOR = 2,
    MOTION = 3,
    FIRE = 4
}AlarmTrigger;

// packad strukt
typedef struct __attribute__((packed))
{
    AlarmTrigger trigger;
    uint32_t time;
}AlarmInfo;

extern AlarmInfo alarmInfo;


// enum: Definierar larm "mode"
typedef enum
{
    STATE_DISARMED,
    STATE_ARMED_HOME,
    STATE_ARMED_AWAY
}AlarmMode;

typedef struct {
    bool wifiIsActive;
    bool bleIsActive;
    bool mqttIsActive;
}ConnectionStatus;

// Struct för all sensor data som samlas in
typedef struct {
    // Alarm (prio 1)
    volatile bool reedSensor1; // volatile: tvingar cpu att läsa variablen från RAM, för att få en 100% korrekt status.
    volatile bool HWEvent_reedSensor1;
    volatile bool motionDetect;
    volatile bool HWEvent_motionDetect;
    

    // Fire (prio 2)
    bool smokeSensor;
    float fireTemp; // over 60c?

    // Enviroment (prio 3)
    float indoorTemp;
    float indoorHumidity;
    bool waterLeak;
}SensorData;


// struct: Definiera "VAD" som larmar.
typedef struct
{
    bool intrusionAlarm;
    bool fireAlarm;
    bool waterLeak;
    bool systemFailure;
}AlarmReason;


// struct: packa SAMTLIG data (extern)
typedef struct
{
    RunStatus runStatus;       // WAKING_UP | RUNNING
    ConnectionStatus connectionStatus; // WiFi Active? | BLE Active? | MQTT Active?
    AlarmMode alarmMode;       // STATE_DISARMED | STATE_ARMED_HOME | STATE_ARMED_AWAY
    AlarmReason alarmStatus;   // intrusionAlarm | fireAlarm | waterLeak | systemFailure
    SensorData sensors;         // all sensordata
    volatile unsigned long sysTime;      // System-tiden
}System;

// deklarera variabel för systemet
extern System node;

#endif