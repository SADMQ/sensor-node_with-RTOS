#ifndef ALARM_H
#define ALARM_H

void checkAlarmStatus();

typedef enum
{
    WAKING_UP,
    RUNNING
}RunStatus;

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
}ConnectionStatus;

// Struct för all sensor data som samlas in
typedef struct {
    // Alarm (prio 1)
    volatile bool reedSensor1; // volatile: tvingar cpu att läsa variablen från RAM, för att få en 100% korrekt status.
    volatile bool reedSensor2;
    volatile bool motionDetect;

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
    RunStatus runStatus;       // WAKING_UP (ca 60 s) | RUNNING
    ConnectionStatus connectionStatus; // WiFi Active? | BLE Active?
    AlarmMode alarmMode;       // Disarmed | Armed home | Armed away
    AlarmReason alarmStatus;   // intrusionAlarm | fireAlarm | waterLeak | systemFailure
    SensorData sensors;         // all sensordata
}System;

// deklarera variabel för systemet
extern System node;

#endif