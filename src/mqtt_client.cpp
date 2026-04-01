#include "alarm.h"
#include "mqtt_client.h"
#include "wifi_manager.h"
#include "certificate.h"
#include <ArduinoMqttClient.h>
//#include <ArduinoJson.h> - används inte ännu..
#include "indicateStatus.h"
#define MQTT_SEND_TIME 30000            // Hur ofta ska vi skicka mqtt.. Testar: 30s
#define MQTT_RECONNECT_TIME 60000       // max reconnect intervall, Testar: 15s
#define MQTT_CONNECTION_TIMEOUT 5000   
#define MQTT_HEARTBEAT 10000            // | Testar 10s (LWT sker ~16s)
#define BROKER_PORT 8883                // okrypt: 1883 - TLS, krypt: 8883

WiFiSSLClient wifiClient;
MqttClient mqttClient(wifiClient);

int port                         = BROKER_PORT;
const char broker[]              = ZeroIP;
const char indoorTempTopic[]     = "sensor/indoorTemp";
const char indoorHumidTopic[]    = "sensor/indoorHumidity";
const char waterleakTopic[]      = "sensor/waterleak";
const char fireTopic[]           = "sensor/fire";
const char systemFailure[]       = "systemFailure";
const char willTopic[]           = "sensor-node-status";
const char willPayload[]         = "OFFLINE";
bool willRetain                  = true;
int willQos                      = 1;

//bool tryMQTTconnect = false;
unsigned long MQTTConnectTimer = -MQTT_RECONNECT_TIME; // Testar: för att connecta omedelbart vid uppstart..
unsigned long MQTTLastSendTimer = -MQTT_SEND_TIME; // Testar: skicka första meddelandet omedelbart..

void initCredentials(){

    wifiClient.setCACert(root_ca);
    mqttClient.setUsernamePassword(MQTT_USER, MQTT_PASS);
}

int manageMQTT() {

    if (node.connectionStatus.mqttIsActive){
        sendMQTT();
        receiveMQTT();
    }

    if ((node.sysTime - MQTTConnectTimer >= MQTT_RECONNECT_TIME) && (!mqttClient.connected())){
        MQTTConnectTimer = node.sysTime;
        
        // testamente
        mqttClient.beginWill(willTopic, willRetain, willQos);
        mqttClient.print(willPayload);
        mqttClient.endWill();

        // mqtt settings
        mqttClient.setKeepAliveInterval(MQTT_HEARTBEAT);
        mqttClient.setConnectionTimeout(MQTT_CONNECTION_TIMEOUT);

        if (mqttClient.connect(broker, port)) { 
            Serial.println("\nMQTT: Connecting..\n");
            node.connectionStatus.mqttIsActive = true;
            initSendMQTT();
            return true;

        } else {
            node.connectionStatus.mqttIsActive = false;
            Serial.println("\nMQTT: Connect error - reconnecting..\n");
            return false;
        }
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
        mqttClient.beginMessage(indoorTempTopic,false, 0,false); // QoS = 0
        mqttClient.print(node.sensors.indoorTemp);
        if (mqttClient.endMessage()) {
            Serial.println("\nMQTT: Temp - Sent!");
        } 

        mqttClient.beginMessage(indoorHumidTopic,false, 0,false);
        mqttClient.print(node.sensors.indoorHumidity);
        if (mqttClient.endMessage()) {
            Serial.println("MQTT: Humidity - Sent!\n");
        } 

        if (node.alarmStatus.fireAlarm){
            mqttClient.beginMessage(fireTopic,false, 0,false);
            mqttClient.print(node.sensors.fireTemp);
            mqttClient.print(node.sensors.smokeSensor);
            if (mqttClient.endMessage()) {
                Serial.println("\nMQTT: Fire - Sent!\n");
        } 

        if (node.alarmStatus.waterLeak){
            mqttClient.beginMessage(waterleakTopic,false, 0,false);
            mqttClient.print(node.sensors.waterLeak);
            if (mqttClient.endMessage()) {
                Serial.println("\nMQTT: Water - Sent!\n");
            } 
        }

        if (node.alarmStatus.systemFailure){
            // skicka releveant larm
        } 
    }
    }
}

void receiveMQTT(){

    // TA EMOT data (sub) från ESP -> "state"
}