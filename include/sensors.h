#ifndef SENSORS_H
#define SENSORS_H

void readPrio3SensorsAsync();

struct SensorData {
    // Alarm (prio 1)
    volatile bool reedSensor1; // volatile: tvingar cpu att läsa variablen från, för att få en 100% korrekt status.
    volatile bool reedSensor2;
    volatile bool motionDetect;

    // Fire (prio 2)
    bool smokeSensor;
    float fireTemp; // over 60c?

    // Enviroment (prio 3)
    float indoorTemp;
    float indoorHumidity;
    bool waterLeak;
};

extern SensorData currentStatus;


#endif