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

// Libraries for Web Services
#include <ESPAsyncTCP.h>       // library used with ESP8266
#include <ESPAsyncWebServer.h>      // library used with ESP32

// Standard web server, on port 80. Must be global. Obvs.
AsyncWebServer server(80);

// Allow OTA software updates
#include <AsyncElegantOTA.h>    // library used with ESP8266 & ESP32

#define SERIAL_BAUD 115200

// Standard ESP8266 Pin defines
#define LED_BUILT_IN_AUX 16   // GPIO16 (D0) *NOTE: Also used for Deep Sleep Wake
#define SDA 4                 // GPIO4 (D2)
#define SCL 5                 // GPIO5 (D1)

#endif