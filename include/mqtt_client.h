#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

void initMQTTid();
void initCredentials();
int manageMQTT();
void initSendReceiveMQTT();
void sendMQTT(AlarmInfo *info);
void receiveMQTT(int messageSize);

#endif