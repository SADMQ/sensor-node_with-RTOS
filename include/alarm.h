#ifndef ALARM_H
#define ALARM_H
#include <Arduino_FreeRTOS.h>

// Semaphore, timers and Queue.
extern SemaphoreHandle_t xAlarmSemaphore;
extern SemaphoreHandle_t xNetworkSemaphore;
extern SemaphoreHandle_t xSystemMonitorSemaphore;
extern TimerHandle_t xAlarmEntryTimer; 
extern TimerHandle_t xHeartbeatTimer;
extern QueueHandle_t xAlarmQueue;
extern QueueHandle_t xMessageQueue; 
extern QueueHandle_t xHeartbeatStateQueue; 

// HIGH PRIO pins to monitor (HW interrupt)
const int reedPin = 3;
const int pirPin = 4;

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
    FIRE = 4,
    LOST_NODE = 5
}AlarmTrigger;

// enum: Definierar larm "mode"
typedef enum : uint8_t
{
    STATE_DISARMED = 0,
    STATE_ARMED_HOME = 1,
    STATE_ARMED_AWAY = 2
}AlarmMode;

typedef struct {
    int16_t inTemp;
    uint16_t inHum;
    int16_t apiTemp; // outdoor temp, from API
    uint16_t apiHum; // outdoor humidity, from API
} Climate;

// packad strukt: för att skicka via BLE - Bara utvald AlarmInfo-data används för JSON objektet, som skickas mot broker.
typedef struct __attribute__((packed)) // 15-byte packat
{
    AlarmMode alarmMode;    // State:   0=Disarmed, 1=ArmedHome, 2=ArmedAway                            [1 byte]
    AlarmTrigger trigger;   // Trigger: 0=None, 1=Water, 2=Door, 3=Motion, 4=Fire, 5=NodeMissing (ESP)  [1 byte]
    uint32_t time;          // Unixtime: Update for state AND trigger                                   [4 byte]
    uint8_t remoteActivate; // Remote activate alarm - from Thingsboard                                 [1 byte]
    Climate climate;        // Indoor & Outdoor (API): temp / humidity                                  [8 byte]
}AlarmInfo;

extern AlarmInfo alarmInfo;

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
    RunStatus runStatus;        // WAKING_UP | RUNNING
    ConnectionStatus connectionStatus; // WiFi Active? | BLE Active? | MQTT Active?
    AlarmMode alarmMode;        // STATE_DISARMED | STATE_ARMED_HOME | STATE_ARMED_AWAY
    AlarmReason alarmStatus;    // intrusionAlarm | fireAlarm | waterLeak | systemFailure
    SensorData sensors;         // all sensordata
    volatile unsigned long sysTime;      // System-tiden
    bool timeIsSet;             // Clock is synchronized
    bool NTCsynced;
}System;

// deklarera variabel för systemet
extern System node;

// -- FUNCTIONS --
int checkAlarmStatus();
void vAlarmTask(void *Params);
void vNetworkTask(void *Params);
void vSystemMonitorTask(void *Params);
void updCurrentTime(char* timestamp, size_t size);
void vBLETask(void *Params);
void dispatchAlarm(bool sharpDoorAlarm = false);
void vAlarmTimerCallback(TimerHandle_t xTimer);
void vHeartbeatMissingCallback(TimerHandle_t xTimer);
void handleStateChange(AlarmMode newState);

#endif