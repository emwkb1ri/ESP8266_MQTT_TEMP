#ifndef __WiFi_INIT_H
#define __WiFi_INIT_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "WiFiSecrets.h"
#include <stdlib.h>

//++++++++++++++++++++++
// WiFi variable definitions
struct Config {   // configuration parameters
  char ssid[50];
  char pw[50];
  long unsigned int wifiTimeout;
  char mqttEnable[3];
  int mqttPort;
  char mqttServer[50];
  char mqttSecureEnable[3];
  char mqttUser[50];
  char mqttPW[50];
  char staticIPenable[3];
  char staticIP[20];
  char staticGatewayAddress[20];
  char staticSubnetAddress[20];
  char staticPrimaryDNSAddress[20];
  char staticSecondaryDNSAddress[20];
};

struct Status {   // status json parameters
  char host[20];
  IPAddress ip;
  IPAddress subnetMask;
  IPAddress gatewayIP;
  byte mac[6];
  char wifi[15];
  int rssi;
};

//++++++++++++++++++++++
// Forward function declarations
bool WiFi_Init();
void getFourNumbersForIP(const char *ipChar);
void array_to_string(byte array[], unsigned int len, char buffer[]);
void loadConfiguration(Config &config);
// Forward declaration: convert Wi-Fi connection response to meaningful message
const char *wl_status_to_string(wl_status_t status);


#endif