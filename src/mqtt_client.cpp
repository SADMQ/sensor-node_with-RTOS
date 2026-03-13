#include "alarm.h"
#include "mqtt_client.h"
#include "wifi_manager.h"
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#define MQTT_SEND_TIME 2000 // skicka MQTT varannan sek
#define MQTT_RECONNECT_TIME 30000 // reconnect, 30s
#define MQTT_CONNECTION_TIMEOUT 1000

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
IPAddress ipReturn; // Needed to ping broker before trying MQTT-connect

int willQos                      = 1;
int port                         = 1883;
const char broker[]              = "192.168.1.100";
const char indoorTempTopic[]     = "sensor/indoorTemp";
const char indoorHumidTopic[]    = "sensor/indoorHumidity";
const char waterleakTopic[]      = "sensor/waterleak";
const char fireTopic[]           = "sensor/fire";
const char systemFailure[]       = "systemFailure";
const char willTopic[]            = "Sensor-node";
const char willPayload[]          = "OFFLINE";
bool willRetain                  = true;

//bool tryMQTTconnect = false;
unsigned long MQTTConnectTimer = 0;
unsigned long MQTTLastSendTimer = 0;


// OBS! "RECONNECT" ÄR INTE IMPLEMENTERAT ÄNNU
// mqttClient.conected() låser systemet under flera sek.. bör bara ske när vi är avlarmade,
// om vi inte hittar något sätt att connecta snabbare.


bool manageMQTT() {

    // d) LWT: Maxtid, 10s offline -> ESP ger larm 
    // skriv testamentet här...
    // 1. Definiera dina inställningar (gör detta innan connect)

    // testamente
    mqttClient.beginWill(willTopic, willRetain, willQos);
    mqttClient.print(willPayload);
    mqttClient.endWill();

    mqttClient.setKeepAliveInterval(15000);                     // heartbeat, 15s
    mqttClient.setConnectionTimeout(MQTT_CONNECTION_TIMEOUT);   // sätter tiden till max 1s

    if ((node.sysTime - MQTTConnectTimer >= MQTT_RECONNECT_TIME) && (!node.connectionStatus.mqttIsActive) ){ 
        MQTTConnectTimer = millis();
            
        if (mqttClient.connect(broker, port)) { 
            node.connectionStatus.mqttIsActive = true;
            initSendMQTT();
            return true;

        } else {
            node.connectionStatus.mqttIsActive = false;
            Serial.println("\n<< MQTT : Connect error >> Try reconnect..\n");
            return false;

        }
    } 

    if (node.connectionStatus.mqttIsActive){
        sendMQTT();
        receiveMQTT();
    }
}

void initSendMQTT(){
        // one-time, init messages
    mqttClient.beginMessage(willTopic, true, 1, false);
    mqttClient.print("ONLINE");
    mqttClient.endMessage();
    }

// -- avgör om datan behöver publiseras - Beroende på sensorer/status samt state --
void sendMQTT(){
    // .poll() : håller igång anslutningen (ping) - och skickar/tar emot MQTT
    mqttClient.poll();

    if (node.sysTime - MQTTLastSendTimer >= MQTT_SEND_TIME){
        MQTTLastSendTimer = node.sysTime;
        mqttClient.beginMessage(indoorTempTopic,false, 0,false);
        mqttClient.print(node.sensors.indoorTemp);
        if (mqttClient.endMessage()) {
            Serial.println("Temp: Sent!");
        } 

        mqttClient.beginMessage(indoorHumidTopic,false, 0,false);
        mqttClient.print(node.sensors.indoorHumidity);
        if (mqttClient.endMessage()) {
            Serial.println("Humidity: Sent!");
        } 

        if (node.alarmStatus.fireAlarm){
            mqttClient.beginMessage(fireTopic,false, 0,false);
            mqttClient.print(node.sensors.fireTemp);
            mqttClient.print(node.sensors.smokeSensor);
            if (mqttClient.endMessage()) {
                Serial.println("Fire: Sent!");
        } 

        if (node.alarmStatus.waterLeak){
            mqttClient.beginMessage(waterleakTopic,false, 0,false);
            mqttClient.print(node.sensors.waterLeak);
            if (mqttClient.endMessage()) {
                Serial.println("Water: Sent!");
            } 
        }

        if (node.alarmStatus.systemFailure){
            // skicka releveant larm
        } 
    }
        // c) skicka heartbeat var 30e sek (bra trots LWT)
    }
}

void receiveMQTT(){


    // TA EMOT data (sub) från ESP -> "state"
}