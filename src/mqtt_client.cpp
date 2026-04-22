#include "alarm.h"
#include "mqtt_client.h"
#include "wifi_manager.h"
#include "certificate.h"
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h> 
#include "indicateStatus.h"
#define MQTT_SEND_TIME 30000           // Hur ofta ska vi skicka mqtt.. Testar: 30s
#define MQTT_RECONNECT_TIME 15000       // max reconnect intervall, Testar: 60s->10s
#define MQTT_CONNECTION_TIMEOUT 20000  // testar öka från 5000..   
#define MQTT_HEARTBEAT 20000           // | Testar 20s (LWT sker ~16s)
#define BROKER_PORT 1883               // okrypt: 1883 - TLS, krypt: 8883

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

int port                         = BROKER_PORT;
const char broker[]              = ZeroIP;
const char alarmInfoTopic[]      = "alarmInfo"; // -> State, Trigger + Time 'struct' .. JSON.

const char indoorTempTopic[]     = "sensors/indoorTemp";         // - Bara för test (ta bort senare..?)
const char indoorHumidTopic[]    = "sensors/indoorHumidity";     // - bara för test (ta bort senare..?)
const char systemFailure[]       = "systemFailure";
const char willTopic[]           = "sensor-node-status";
const char willPayload[]         = "OFFLINE";
bool willRetain                  = true;
int willQos                      = 1;

//bool tryMQTTconnect = false;
unsigned long MQTTConnectTimer = -MQTT_RECONNECT_TIME; // Testar: för att connecta omedelbart vid uppstart..
unsigned long MQTTLastSendTimer = -MQTT_SEND_TIME; // Testar: skicka första meddelandet omedelbart..

void initCredentials(){

    //wifiClient.setCACert(root_ca); // ----------- <<< OBS: INAKTIVERAR FÖR TEST <<<
    //mqttClient.setUsernamePassword(MQTT_USER, MQTT_PASS);
}

int manageMQTT() {

    if (node.connectionStatus.mqttIsActive){
        
        
        sendMQTT(nullptr); // ---> Flyttad! Ligger i task, men nullptr körs ändå för poll. ( samt för DHT11 än så länge ).
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
void sendMQTT(AlarmInfo *info){
    // .poll() : håller igång anslutningen (ping) - och skickar/tar emot MQTT
    mqttClient.poll();
    
    if (info != nullptr){
        // Skapa JSON doc 
        JsonDocument doc;

        // Mappa struct till JSON-nycklar
        doc["id"] = "SENSOR_NODE"; 
        doc["trigger"] = info->trigger;
        doc["time"] = info->time;
        doc["state"] = (uint8_t)node.alarmMode;

        // Konvertera till en sträng
        String jsonString;
        serializeJson(doc, jsonString);

        // Skicka via din MQTT-klient
        mqttClient.beginMessage(alarmInfoTopic, false, 1, false);
        //mqttClient.print(jsonString); -- kan tas bort..
        
        // skriver direkt till streamen istället för via en sträng.. för optimering
        serializeJson(doc, mqttClient); 

        if (mqttClient.endMessage()){
            Serial.println("MQTT skickat: " + jsonString);
        } else {
            Serial.println("! MQTT paket kunde ej skickas !");
        }
    }
    
    if (node.sysTime - MQTTLastSendTimer >= MQTT_SEND_TIME){
        MQTTLastSendTimer = node.sysTime;
        mqttClient.beginMessage(indoorTempTopic,false, 0,false); // QoS = 0
        mqttClient.print(node.sensors.indoorTemp);
        if (mqttClient.endMessage()) {
            Serial.println("\nMQTT: Temp - Sent!");
        } 
    }
//
    //    mqttClient.beginMessage(indoorHumidTopic,false, 0,false);
    //    mqttClient.print(node.sensors.indoorHumidity);
    //    if (mqttClient.endMessage()) {
    //        Serial.println("MQTT: Humidity - Sent!\n");
    //    } 

        if (node.alarmStatus.systemFailure){
            // skicka releveant larm
        } 
}
    

void receiveMQTT(){
    // TA EMOT data (sub) från Broker -- "remoteActivate"
}