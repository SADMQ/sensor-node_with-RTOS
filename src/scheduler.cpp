// --- definierar NÄR resp. funktion ska köras ---
#include <stdio.h>
#include "tasks.h"

// enum för resp. PRIO-klass.
enum TaskType {sensorPrio1, sensorPrio2, sensorPrio3, servicePrio1, servicePrio2};

// struct för resp. TASK, innehållande: Prioklass/funktion - intervall (ms) - senaste körningen (ms)
struct Tasks {
    TaskType prioType;
    unsigned long intervall;
    unsigned long lastRun;
};

// Tasks-array: "taskList" -> innehåller samtliga TASKS. ( utelämnar lastRun->0) )
Tasks taskList[] = {
    {sensorPrio1, 20},  // Provar med 20ms.
    {sensorPrio2, 500},
    {sensorPrio3, 2000},
    {servicePrio1, 100}, // skicka larm vid triggning -> checkAlarmTrigger() ?
    {servicePrio2, 2000}, // håll igång WiFi/BLE?
};

void taskScheduler(){

  // Loopa igenom listan med TASKS (taskList).
  // Kolla om den nuvarande TASKEN bör köras - alltså: är nuvarande tid MINUS lastrun mer eller lika med intervall?
  // Om TRUE -> kör
      // switch-case -> Prio3=readPrio3Sensors();
  // Om FALSE -> loopa vidare

  readPrio3Sensors(); // läs (Prio 3) sensorer -> var 2:e sek? * 2.

  // 
  // om larm triggats - skicka till ESP (via BLE)


};

