// main header file
#ifndef __MAIN_H
#define __MAIN_H

// Local includes
#include "WiFi_Init.h"
#include "OTA_Init.h"

// Library includes required for this program
#include <Arduino.h>    // required for VSC & PlatformIO
#include <Wire.h>       // common library for I2C devices
#include <ESP8266WiFi.h> // for NodeMCU and ESP8266 ethernet modules
#include <ESP8266WiFiMulti.h>
// Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WiFiMulti wifiMulti;
#include <PubSubClient.h>   // MQTT client library header

// Libraries for Web Services
#include <ESPAsyncTCP.h>       // library used with ESP8266
#include <ESPAsyncWebServer.h>      // library used with ESP32

// Standard web server, on port 80. Must be global. Obvs.
AsyncWebServer server(80);

// Allow OTA software updates
#include <AsyncElegantOTA.h>    // library used with ESP8266 & ESP32

#define SERIAL_BAUD 115200

// Standard ESP8266 Pin defines
#define LED_BUILT_IN_AUX 16     // GPIO16(D0) *NOTE: Also used for Deep Sleep Wake
#define RTC_RESET 16            // signal RTC_RESET GPIO16(D0) Deep Sleep Wake
#define SDA 4                   // GPIO4 (D2)
#define SCL 5                   // GPIO5 (D1)
#define GPIO0 0                 // GPIO0 (D3)
#define RELAY 0                 // GPIO0 (D3)
#define GPIO2 2                 // GPIO2 (D4)
#define GPIO14 14               // GPIO14 (D5)
#define NOT_NO_SLEEP 14         // signal ^NO_SLEEP on GPIO 14 (D5)

//+++++++++++++++++++++++++++++
// analog pin configuration
// battery voltage divider = 100K/540K = 0.1852
// max 1V range = 1/.1852 = 5.3995
// 1024 steps - 0.0053 volts/step
// actual measured is closer to 0.0051 or 5.15/1024
// ADC_MODE(ADC_VCC);
const float volts_per_step = 5.15/1024;

//++++++++++++++++++++++++++++++++++++
// One Wire bus and temperature probe libraries
#include <OneWire.h>
#include <DallasTemperature.h>
// temperature sensor defines and variables
#define ONE_WIRE_BUS 2        // GPIO2 (D4) One Wire bus interface
#define TEMP_INTERVAL 10000UL
#define MAX_DEVICES 5
int numDevices = 0;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress tempSensor[MAX_DEVICES];

//++++++++++++++++
// deep sleep variables
#define SLEEP_TIME_SIXTY_SECONDS 60*1000000UL
#define SLEEP_TIME_TEN_MINUTES 600*1000000UL
#define SLEEP_TIME_THIRTY_MINUTES 1800*1000000UL
bool sleep;  // true = deep sleep, false = no sleep
unsigned long runTimer = 0;

// MQTT defines
#define STATUS_INTERVAL 30000UL   // online message interval
#define QOS_0 0
#define QOS_1 1
#define QOS_2 2

extern Config config;  // declare the external configuration struct
extern Status status;  // declare the external status struct
extern WiFiClient wifiClient;   // declare the external WiFiClient object
extern PubSubClient mqttClient;    // declare the external PubSubClient object

extern const char topicPreamble[];
extern const char will[];
extern int willQoS;         // use QoS = 1 for last will message
extern bool willRetain;     // retain the last will
extern bool cleanSession;   // true = start fresh; false = durable
extern const char cmd[];
extern const char statusTopic[];
extern char willTopic[40];
extern char inTopic[40];
extern char outTopic[40];
extern char willMessage[128];
extern char outMsg[128];
extern char rcvMsg[256];    //message receive buffer
extern char rcvTopic[128];  // rcvTopic buffer
extern bool newMsgFlag;     // new subscribed message received
// milliseconds default = 15 * 1000L
extern const unsigned long keepAlive;
// milliseconds default = 30 * 1000L
extern const unsigned long timeout;

// Forward function definitions
extern bool connectMqtt();
extern void mqttTopicInit();
extern void mqttCallback(char* topic, byte* payload, unsigned int length);
extern int mqttState();
extern void publish(char* topic, char* msg);

// forward function definitions
void oneWireInit();
void elegantOTA_Init();
float printTemperature(DeviceAddress deviceAddress);
void printAddress(DeviceAddress deviceAddress);
void publishMsg1(char msg[]);
void publishTemps(char msg[], int devices);
void updateRunTime();
unsigned long getSavedRunTime();


#endif