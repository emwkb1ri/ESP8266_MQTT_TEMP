// header file guard statements to prevent duplicate defines
#ifndef __WIFI_SECRETS_H__
#define __WIFI_SECRETS_H__

/* Template file for your personal IP addresses
 * Rename this file to WiFiSecrets.h
 * if using GIT - add WiFiSecets.h to your .gitignore configuration
 * Be careful not to inadvertently share your network passwords
 *
 * Use this file in conjunction with configuration.ino or WiFi_Init.cpp
 * Your sketch or sketch header file should contain the following statement
 * "#include WiFiSecrets.h"
 */

#define WIFI_TIMEOUT 20000UL    // connection timeout in milliseconds

// My WiFi SSID & Password
#define SECRET_SSID "Your-Network-SSID"
#define SECRET_PASS "Your-Network-Passcode"

// alternate SSID optional for WiFiMulti library
//#define SECRET_SSID1 "Your-alternate-SSID"
//#define SECRET_PASS1 "Your-alternate-Passcode"

// OTA Password
#define SECRET_OTA_PASS "protoNR4O"

// My list of key IP addresses
#define STATIC_IP_ENABLED "f"         // enable Static IP "t" or "f" in quotes
#define STATIC_IP "Your-Static-IP"    // optional
#define SUBNET_MASK "255.255.255.0"   // most common mask
#define GATEWAY_IP "Your-Gateway-IP"

// Primary and secondary DNS addresses
#define PRIMARY_DNS_IP "Primary-DNS-IP"
#define SECONDARY_DNS_IP "Secondary-DNS-IP"

// MQTT IP Address and port
#define MQTT_ENABLED "f"  // enable MQTT "t" or "f" in quotes
#define MQTT_SERVER_IP "Your-MQTT-Server-IP"
#define MQTT_PORT 1883    // default MQTT port - integer type
#define MQTT_SECURE_ENABLE "f"  //secure MQTT "t" or "f" in quotes
#define MQTT_USER ""      // default null or none
#define MQTT_PASSWORD ""  // default null or none

#endif
