#ifndef __MQTT_H__
#define __MQTT_H__

#include <Arduino.h>        // required for VSC & PlatformIO
#include <ESP8266WiFi.h>    // for NodeMCU and ESP8266 ethernet modules
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>   //for mqtt
#include "WiFi_Init.h"      // needed for status struct
#include <stdlib.h>


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);    // create a PubSubClient object

//++++++++++++++++++++++
// MQTT function globals
#define STATUS_INTERVAL 30000UL   // online message interval
// MQTT topic definitions
#define QOS_0 0
#define QOS_1 1
#define QOS_2 2
const char topicPreamble[] = "MyIoT/";
const char will[] = "/will";
int willQoS = 1;                  // use QoS = 1 for last will message
bool willRetain = true;           // retain the last will
bool cleanSession = true;         // true = start fresh; false = durable
const char cmd[] = "/cmd";
const char statusTopic[] = "/status";
char willTopic[40] = "/ESP_xxxxxx/will";
char inTopic[40]   = "/ESP_xxxxxx/cmd";
char outTopic[40] = "/ESP_xxxxxx/status";
char willMessage[128] = "Offline";
char outMsg[128] = "Online";
char rcvMsg[256] = "";  //message receive buffer
char rcvTopic[128] = ""; // rcvTopic buffer
bool newMsgFlag = false;  // new subscribed message received
// milliseconds default = 15 * 1000L
const unsigned long keepAlive = 15 * 1000UL;
// milliseconds default = 30 * 1000L
const unsigned long timeout = 15 * 1000UL;

// Forward function definitions
bool connectMqtt();
void mqttTopicInit();
void mqttCallback(char* topic, byte* payload, unsigned int length);
int mqttState();
void publish(char* topic, char* msg);

#endif  // __MQTT_H__
