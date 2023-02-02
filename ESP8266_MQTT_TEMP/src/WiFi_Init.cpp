#include "WiFi_Init.h"

Config config;  // instantiate the configuration struct
Status status;  // instantiate the status struct

char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
char rootHostname[20] = "ESP_";  // set root mqtt hostname of device
String hostname; // default hostname of this module as char array
//char myHostname[20]; // char array for generated hostname at WiFi connect
//byte mac[6];  // the MAC address of your WiFi Module
//IPAddress ip; // the IP address of this board
//IPAddress subnetMask; // subnet mask
//IPAddress gatewayIP; // IP address of the gateway

bool wifiConnected = false;
int oneIP = 0, twoIP = 0, threeIP = 0, fourIP = 0;


/* ++++++++++ Not used with ESP8266 +++++++++++++++
//wifi event handler

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //When connected set
      wifiConnected = true;
      break;
    //    case SYSTEM_EVENT_STA_DISCONNECTED:
    //      Serial.println("WiFi lost connection");
    //      connected = false;
    //      break;
    default: break;
  }
}
*/

bool WiFi_Init() {
    // load network configuration parameters
  loadConfiguration(config);

  Serial.printf("...WiFi timeout=%lu...\n", config.wifiTimeout);

  //not used for basic ESP-01 DHCP connection at this time
  if (strcmp(config.staticIPenable, "t") == 0) {//if static
    Serial.println("Setting up Static IP");
    getFourNumbersForIP(config.staticIP);
    Serial.printf("%i.%i.%i.%i\n", oneIP, twoIP, threeIP, fourIP);
    IPAddress ip(oneIP, twoIP, threeIP, fourIP);
    getFourNumbersForIP(config.staticGatewayAddress);
    Serial.printf("%i.%i.%i.%i\n", oneIP, twoIP, threeIP, fourIP);
    IPAddress gateway(oneIP, twoIP, threeIP, fourIP);
    getFourNumbersForIP(config.staticSubnetAddress);
    Serial.printf("%i.%i.%i.%i\n", oneIP, twoIP, threeIP, fourIP);
    IPAddress subnet(oneIP, twoIP, threeIP, fourIP);
    getFourNumbersForIP(config.staticPrimaryDNSAddress);
    Serial.printf("%i.%i.%i.%i\n", oneIP, twoIP, threeIP, fourIP);
    IPAddress primaryDNS(oneIP, twoIP, threeIP, fourIP);
    getFourNumbersForIP(config.staticSecondaryDNSAddress);
    Serial.printf("%i.%i.%i.%i\n", oneIP, twoIP, threeIP, fourIP);
    IPAddress secondaryDNS(oneIP, twoIP, threeIP, fourIP);
    WiFi.config(ip, gateway, subnet, primaryDNS, secondaryDNS);
  }
  Serial.printf("%s\n", config.ssid);


  wifiConnected = false;
  //WiFi.onEvent(WiFiEvent);
  WiFi.macAddress(status.mac);
  char str[10] = "";
  char buf[30];

  #ifdef ESP8266
    WiFi.mode(WIFI_STA);  // needed for ESP8266WiFi
    byte array[3] = { status.mac[3], status.mac[4], status.mac[5] };
    // put the mac address into the status struct
  #else
    byte array[3] = { status.mac[2], status.mac[1], status.mac[0] };
  #endif

  unsigned long wifiStart = millis();
  WiFi.begin(config.ssid, config.pw);
  while (WiFi.status() != WL_CONNECTED) {
    // check for timeout
      delay(500);
      Serial.print(".");

    if (millis() - wifiStart > config.wifiTimeout) {
      Serial.printf("...ERROR - Connection Timeout - %lumS...\n", millis() - wifiStart);
      //killPower();  // not using this for initial version
      wl_status_t reply = WiFi.status();
      Serial.println(wl_status_to_string(reply));
      ESP.restart();    // connection failed - restart device and retry
      return false;
    }
  }
  // print how long it took to connect
  Serial.printf("%lumS\n", millis() - wifiStart);
  wifiConnected = true;

  // Generate a Hostname for this device based on status.host plus the last byte of the MAC address
  strcpy(status.host, rootHostname);  // set the root string for status.host
  strcpy(buf, status.host);
  //strcat(buf, ":");

  array_to_string(array, 3, str);  // convert the last 3 bytes of MAC address to a string
  strcat(buf, str);
  //Serial.println("");
  Serial.println(buf);

  // print the SSID you are connected to
  Serial.print("CONNECTED TO: ");
  Serial.println(WiFi.SSID());
  // print the signal strength of the WiFi
  Serial.print("Signal Strength (RSSI): ");
  status.rssi = (WiFi.RSSI());
  Serial.println(WiFi.RSSI());

  #ifdef ESP8266
  // hostname function only available on ESP8266WiFi
  hostname = (WiFi.hostname());  // get the current Hostname
  strcpy(status.host, buf);
  #else
  hostname = "NOT READABLE";
  strcpy(status.host, buf);
  #endif

  Serial.print("Hostname: ");
  Serial.print(hostname);
  Serial.print("  host: ");
  Serial.println(status.host);

  // print the MAC address beginning with byte 5 to byte 0
  Serial.print("MAC: ");
  #ifdef ESP8266
  // swap order of bytes for ESP8266 modules
    for (unsigned int i = 0; i < 5; i++)
  #else
  // use this ordering for all others like MKR1010
    for (unsigned int i = 5; i > 0; i--)
  #endif

    {
    Serial.print(status.mac[i], HEX);
    Serial.print(":");
    }

  #ifdef ESP8266
    Serial.println(status.mac[5], HEX);
  #else
    Serial.println(status.mac[0], HEX);
  #endif

  status.ip = WiFi.localIP();
  status.subnetMask = WiFi.subnetMask();  // get current subnet mask
  status.gatewayIP = WiFi.gatewayIP();    // get current gateway IP

  // print this device's ip, gateway and subnetmask
  Serial.print("IP: ");
  Serial.println(status.ip);
  Serial.print("Gateway: ");
  Serial.println(status.gatewayIP);
  Serial.print("Subnet Mask: ");
  Serial.println(status.subnetMask);

  return true;
}

/*--------------------------------------------------------------------------
 * Translates the Wi-Fi connect response to English
 *------------------------------------------------------------------------*/
const char *wl_status_to_string(wl_status_t status)
{
  switch (status) {
    case WL_NO_SHIELD:
      return "WL_NO_SHIELD";
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "UNKNOWN";
  }
}

/*--------------------------------------------------------------------------
 * Function to convert byte array to HEX string
 *------------------------------------------------------------------------*/
void array_to_string(byte array[], unsigned int len, char buffer[]) {
  // converts a byte array to a hex character string that can be printed
  // args are:
  //      byte array to be converted
  //      length of array
  //      destination char buffer

    for (unsigned int i = 0; i < len; i++){
        byte nib1 = (array[i] >> 4) & 0x0F;  // mask the first nibble of data in element i of array
        byte nib2 = (array[i] >> 0) & 0x0F;  // mask the second nibble of data in element i of array
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;  // convert the nibble to the ASCII character '0' to 'F'
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

/*-------------------------------------------------------------------------
 * Function to convert an IP address string constant to four numbers
 *-------------------------------------------------------------------------*/
void getFourNumbersForIP(const char *ipChar) {
  char ipCharInput[20];
  strncpy(ipCharInput, ipChar, sizeof(ipCharInput));

  const char delimiter[] = ".";
  char parsedStrings[4][4];
  char *token =  strtok(ipCharInput, delimiter);
  if (token == NULL)
    return;
  strncpy(parsedStrings[0], token, sizeof(parsedStrings[0]));//first one
  for (int i = 1; i < 4; i++) {
    token =  strtok(NULL, delimiter);
    if (token == NULL)
      return;
    strncpy(parsedStrings[i], token, sizeof(parsedStrings[i]));
  }
  oneIP = atoi(parsedStrings[0]);
  twoIP = atoi(parsedStrings[1]);
  threeIP = atoi(parsedStrings[2]);
  fourIP = atoi(parsedStrings[3]);
}

// scaled down version of SPIFFS configuration struct setup

void loadConfiguration(Config &config) {

  // Copy values from the JsonDocument to the Config
  strncpy(config.ssid,                  // <- destination
          SECRET_SSID,                  // <- source
          sizeof(config.ssid));         // <- destination's capacity
  //Serial.println(config.ssid);

  strncpy(config.pw,                    // <- destination
          SECRET_PASS,                  // <- source
          sizeof(config.pw));           // <- destination's capacity

  config.wifiTimeout = WIFI_TIMEOUT;

  strncpy(config.mqttEnable,                  // <- destination
          MQTT_ENABLED,                       // <- source
          sizeof(config.mqttEnable));         // <- destination's capacity

  config.mqttPort = MQTT_PORT;

  strncpy(config.mqttServer,                  // <- destination
          MQTT_SERVER_IP,                     // <- source
          sizeof(config.mqttServer));         // <- destination's capacity
  strncpy(config.mqttSecureEnable,            // <- destination
          MQTT_SECURE_ENABLE,                 // <- source
          sizeof(config.mqttSecureEnable));   // <- destination's capacity
  strncpy(config.mqttUser,                    // <- destination
          MQTT_USER,                          // <- source
          sizeof(config.mqttUser));           // <- destination's capacity
  strncpy(config.mqttPW,                      // <- destination
          MQTT_PASSWORD,                      // <- source
          sizeof(config.mqttPW));             // <- destination's capacity
  strncpy(config.staticIPenable,              // <- destination
          STATIC_IP_ENABLED,                  // <- source
          sizeof(config.staticIPenable));     // <- destination's capacity
  strncpy(config.staticIP,                    // <- destination
          STATIC_IP,                          // <- source
          sizeof(config.staticIP));           // <- destination's capacity
  strncpy(config.staticGatewayAddress,            // <- destination
          GATEWAY_IP,                             // <- source
          sizeof(config.staticGatewayAddress));   // <- destination's capacity
  strncpy(config.staticSubnetAddress,             // <- destination
          SUBNET_MASK,                            // <- source
          sizeof(config.staticSubnetAddress));      // <- destination's capacity
  strncpy(config.staticPrimaryDNSAddress,           // <- destination
          PRIMARY_DNS_IP,                           // <- source
          sizeof(config.staticPrimaryDNSAddress));  // <- destination's capacity
  strncpy(config.staticSecondaryDNSAddress,         // <- destination
          SECONDARY_DNS_IP,                         // <- source
          sizeof(config.staticSecondaryDNSAddress));// <- destination's capacity

}
/*  ++++++ Don't need all the JsonDocument stuff yet ++++++++++
void loadConfiguration(const char *filename, Config &config) {

  Serial.println(F("checking config file..."));
  if (SPIFFS.begin(true)) {
    File file = SPIFFS.open(filename, "r");

    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      Serial.println(F("Failed to read file"));
    }

    // Copy values from the JsonDocument to the Config
    strlcpy(config.ssid,                  // <- destination
            doc["ssid"] | "your ssid",  // <- source
            sizeof(config.ssid));         // <- destination's capacity
    //Serial.println(config.ssid);

    strlcpy(config.pw,                  // <- destination
            doc["pw"] | "your pw",  // <- source
            sizeof(config.pw));         // <- destination's capacity

    config.wifiTimeout = doc["wifiTimeout"] | 5000;

    strlcpy(config.trigName,                  // <- destination
            doc["trigName"] | "trigBoard Name",  // <- source
            sizeof(config.trigName));         // <- destination's capacity

    strlcpy(config.trigSelect,                  // <- destination
            doc["trigSelect"] | "Both",  // <- source
            sizeof(config.trigSelect));         // <- destination's capacity

    strlcpy(config.triggerOpensMessage,                  // <- destination
            doc["triggerOpensMessage"] | "Contact has Opened",  // <- source
            sizeof(config.triggerOpensMessage));         // <- destination's capacity

    strlcpy(config.triggerClosesMessage,                  // <- destination
            doc["triggerClosesMessage"] | "Contact has Closed",  // <- source
            sizeof(config.triggerClosesMessage));         // <- destination's capacity

    strlcpy(config.buttonMessage,                  // <- destination
            doc["buttonMessage"] | "Button Was Pressed",  // <- source
            sizeof(config.buttonMessage));         // <- destination's capacity

    config.timerCountDown = doc["timerCountDown"] | 5;

    strlcpy(config.timerSelect,                  // <- destination
            doc["timerSelect"] | "Either",  // <- source
            sizeof(config.timerSelect));         // <- destination's capacity

    strlcpy(config.StillOpenMessage,                  // <- destination
            doc["StillOpenMessage"] | "Contact Still Open",  // <- source
            sizeof(config.StillOpenMessage));         // <- destination's capacity

    strlcpy(config.StillClosedMessage,                  // <- destination
            doc["StillClosedMessage"] | "Contact Still Closed",  // <- source
            sizeof(config.StillClosedMessage));         // <- destination's capacity


    config.batteryThreshold = doc["batteryThreshold"] | 2500;
    config.batteryThreshold = config.batteryThreshold / 1000; //convert to volts

    config.batteryOffset = doc["batteryOffset"] | 0;
    config.batteryOffset = config.batteryOffset / 1000; //convert to volts

    strlcpy(config.pushUserKey,                  // <- destination
            doc["pushUserKey"] | "Your User Key",  // <- source
            sizeof(config.pushUserKey));         // <- destination's capacity
    strlcpy(config.pushAPIKey,                  // <- destination
            doc["pushAPIKey"] | "Your API Key",  // <- source
            sizeof(config.pushAPIKey));         // <- destination's capacity
    strlcpy(config.pushOverEnable,                  // <- destination
            doc["pushOverEnable"] | "f",  // <- source
            sizeof(config.pushOverEnable));         // <- destination's capacity
    strlcpy(config.pushSaferEnable,                  // <- destination
            doc["pushSaferEnable"] | "f",  // <- source
            sizeof(config.pushSaferEnable));         // <- destination's capacity
    strlcpy(config.iftttEnable,                  // <- destination
            doc["iftttEnable"] | "f",  // <- source
            sizeof(config.iftttEnable));         // <- destination's capacity
    strlcpy(config.udpEnable,                  // <- destination
            doc["udpEnable"] | "t",  // <- source
            sizeof(config.udpEnable));         // <- destination's capacity
    strlcpy(config.tcpEnable,                  // <- destination
            doc["tcpEnable"] | "f",  // <- source
            sizeof(config.tcpEnable));         // <- destination's capacity
    config.udpPort = doc["udpPort"] | 1234;
    strlcpy(config.pushSaferKey,                  // <- destination
            doc["pushSaferKey"] | "your pushSafer key",  // <- source
            sizeof(config.pushSaferKey));         // <- destination's capacity
    strlcpy(config.iftttMakerKey,                  // <- destination
            doc["iftttMakerKey"] | "your maker key",  // <- source
            sizeof(config.iftttMakerKey));         // <- destination's capacity
    strlcpy(config.udpStaticIP,                  // <- destination
            doc["udpStaticIP"] | "192.168.4.2",  // <- source
            sizeof(config.udpStaticIP));         // <- destination's capacity
    strlcpy(config.udpTargetIP,                  // <- destination
            doc["udpTargetIP"] | "192.168.4.1",  // <- source
            sizeof(config.udpTargetIP));         // <- destination's capacity
    strlcpy(config.udpGatewayAddress,                  // <- destination
            doc["udpGatewayAddress"] | "192.168.4.1",  // <- source
            sizeof(config.udpGatewayAddress));         // <- destination's capacity
    strlcpy(config.udpSubnetAddress,                  // <- destination
            doc["udpSubnetAddress"] | "255.255.0.0",  // <- source
            sizeof(config.udpSubnetAddress));         // <- destination's capacity
    strlcpy(config.udpPrimaryDNSAddress,                  // <- destination
            doc["udpPrimaryDNSAddress"] | "8.8.8.8",  // <- source
            sizeof(config.udpPrimaryDNSAddress));         // <- destination's capacity
    strlcpy(config.udpSecondaryDNSAddress,                  // <- destination
            doc["udpSecondaryDNSAddress"] | "8.8.4.4",  // <- source
            sizeof(config.udpSecondaryDNSAddress));         // <- destination's capacity
    strlcpy(config.udpSSID,                  // <- destination
            doc["udpSSID"] | "your_udp_SSID",  // <- source
            sizeof(config.udpSSID));         // <- destination's capacity
    strlcpy(config.udpPW,                  // <- destination
            doc["udpPW"] | "your_udp_PW",  // <- source
            sizeof(config.udpPW));         // <- destination's capacity
    strlcpy(config.rtcCountdownMinute,                  // <- destination
            doc["rtcCountdownMinute"] | "f",  // <- source
            sizeof(config.rtcCountdownMinute));         // <- destination's capacity
    strlcpy(config.mqttEnable,                  // <- destination
            doc["mqttEnable"] | "f",  // <- source
            sizeof(config.mqttEnable));         // <- destination's capacity
    config.mqttPort = doc["mqttPort"] | 1883;
    strlcpy(config.mqttServer,                  // <- destination
            doc["mqttServer"] | "192.168.1.117",  // <- source
            sizeof(config.mqttServer));         // <- destination's capacity
    strlcpy(config.mqttTopic,                  // <- destination
            doc["mqttTopic"] | "trigBoardTopic",  // <- source
            sizeof(config.mqttTopic));         // <- destination's capacity
    strlcpy(config.mqttSecureEnable,                  // <- destination
            doc["mqttSecureEnable"] | "t",  // <- source
            sizeof(config.mqttSecureEnable));         // <- destination's capacity
    strlcpy(config.mqttUser,                  // <- destination
            doc["mqttUser"] | "User Name",  // <- source
            sizeof(config.mqttUser));         // <- destination's capacity
    strlcpy(config.mqttPW,                  // <- destination
            doc["mqttPW"] | "password",  // <- source
            sizeof(config.mqttPW));         // <- destination's capacity
    strlcpy(config.staticIPenable,                  // <- destination
            doc["staticIPenable"] | "f",  // <- source
            sizeof(config.staticIPenable));         // <- destination's capacity
    strlcpy(config.staticIP,                  // <- destination
            doc["staticIP"] | "192.168.1.200",  // <- source
            sizeof(config.staticIP));         // <- destination's capacity
    strlcpy(config.staticGatewayAddress,                  // <- destination
            doc["staticGatewayAddress"] | "192.168.1.1",  // <- source
            sizeof(config.staticGatewayAddress));         // <- destination's capacity
    strlcpy(config.staticSubnetAddress,                  // <- destination
            doc["staticSubnetAddress"] | "255.255.0.0",  // <- source
            sizeof(config.staticSubnetAddress));         // <- destination's capacity
    strlcpy(config.staticPrimaryDNSAddress,                  // <- destination
            doc["staticPrimaryDNSAddress"] | "8.8.8.8",  // <- source
            sizeof(config.staticPrimaryDNSAddress));         // <- destination's capacity
    strlcpy(config.staticSecondaryDNSAddress,                  // <- destination
            doc["staticSecondaryDNSAddress"] | "8.8.4.4",  // <- source
            sizeof(config.staticSecondaryDNSAddress));         // <- destination's capacity

    strlcpy(config.highSpeed,                  // <- destination
            doc["highSpeed"] | "f",  // <- source
            sizeof(config.highSpeed));         // <- destination's capacity

    config.udpBlastCount = doc["udpBlastCount"] | 10;
    config.udptimeBetweenBlasts = doc["udptimeBetweenBlasts"] | 10;

    file.close();
  } else {
    Serial.println(F("SPIFFS fault"));
    killPower();
  }
}
*/

/*  +++++++Not needed right now +++++++++++
void saveConfiguration(const char *filename, const Config &config) {

  Serial.println(F("saving.."));
  if (SPIFFS.begin(true)) {
    File file = SPIFFS.open(filename, "w");
    if (!file) {
      Serial.println(F("Failed to create file"));
      return;
    }
    StaticJsonDocument<2000> doc;

    // Set the values in the document
    doc["ssid"] = config.ssid;
    doc["pw"] = config.pw;
    doc["wifiTimeout"] = config.wifiTimeout;
    doc["trigName"] = config.trigName;
    doc["trigSelect"] = config.trigSelect;
    doc["triggerOpensMessage"] = config.triggerOpensMessage;
    doc["triggerClosesMessage"] = config.triggerClosesMessage;
    doc["buttonMessage"] = config.buttonMessage;
    doc["timerCountDown"] = config.timerCountDown;
    doc["timerSelect"] = config.timerSelect;
    doc["StillOpenMessage"] = config.StillOpenMessage;
    doc["StillClosedMessage"] = config.StillClosedMessage;

    doc["batteryThreshold"] = config.batteryThreshold * 1000;
    doc["batteryOffset"] = config.batteryOffset * 1000;

    doc["pushUserKey"] = config.pushUserKey;
    doc["pushAPIKey"] = config.pushAPIKey;
    doc["pushOverEnable"] = config.pushOverEnable;
    doc["pushSaferEnable"] = config.pushSaferEnable;
    doc["pushSaferKey"] = config.pushSaferKey;
    doc["iftttEnable"] = config.iftttEnable;
    doc["iftttMakerKey"] = config.iftttMakerKey;
    doc["udpEnable"] = config.udpEnable;
    doc["tcpEnable"] = config.tcpEnable;
    doc["udpTargetIP"] = config.udpTargetIP;
    doc["udpStaticIP"] = config.udpStaticIP;
    doc["udpGatewayAddress"] = config.udpGatewayAddress;
    doc["udpSubnetAddress"] = config.udpSubnetAddress;
    doc["udpPrimaryDNSAddress"] = config.udpPrimaryDNSAddress;
    doc["udpSecondaryDNSAddress"] = config.udpSecondaryDNSAddress;
    doc["udpPort"] = config.udpPort;
    doc["udpPW"] = config.udpPW;
    doc["udpSSID"] =  config.udpSSID;
    doc["rtcCountdownMinute"] =  config.rtcCountdownMinute;
    doc["mqttEnable"] =  config.mqttEnable;
    doc["mqttPort"] =  config.mqttPort;
    doc["mqttServer"] =  config.mqttServer;
    doc["mqttTopic"] =  config.mqttTopic;
    doc["mqttSecureEnable"] =  config.mqttSecureEnable;
    doc["mqttUser"] =  config.mqttUser;
    doc["mqttPW"] =  config.mqttPW;
    doc["staticIPenable"] =  config.staticIPenable;
    doc["staticIP"] =  config.staticIP;
    doc["staticGatewayAddress"] =  config.staticGatewayAddress;
    doc["staticSubnetAddress"] =  config.staticSubnetAddress;
    doc["staticPrimaryDNSAddress"] =  config.staticPrimaryDNSAddress;
    doc["staticSecondaryDNSAddress"] =  config.staticSecondaryDNSAddress;
    doc["highSpeed"] =  config.highSpeed;
    doc["udpBlastCount"] =  config.udpBlastCount;
    doc["udptimeBetweenBlasts"] =  config.udptimeBetweenBlasts;

    // Serialize JSON to file

    if (serializeJson(doc, file) == 0) {
      Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();


  }


}
*/
