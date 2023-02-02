/**********************************************************************
 * Copyright Wagner Enterprises, LLC 2023 - All Rights Reserved
 *
 * @author Eric Wagner
 *
 * Description:
 * ------------
 * This is a PlatformIO project for a ESP8266 based program to monitor
 * temperature of Dallas Semiconductor DS18B20 sensor using a One Wire
 * bus.  It also includes a standard I2C bus configured on GPIO4 and 
 * GPIO5.  Finally it also supports the Arduino OTA and the ElegantOTA
 * network update options.  ElegantOTA can be accessed via the device's
 * IP address/update from any web browser.
 * 
 * This program will also be configured to use deep sleep when operating 
 * on battery power.  One GPIO pin will be configured for external power
 * detection.  Another GPIO pin will monitor the +5Vcc input.  The A0
 * analog input will monitor battery voltage.  
 *
 * Temperature, battery voltage and power supply state will be reported
 * via MQTT to my local Mosquitto broker.
 *
 * Programming setup for the ESP8266: NodeMCU ESP8266(12E),
 *                                    Flash 4MB(FS:2 MB,OTA:~1 MB)
 *                                    Flash Frequency: 80MHz
 *
 * Revision History:
 * -----------------
 * 0.00 - Base program with simple WiFi connect, I2C, OTA and LED blink
 * 0.01 - Changes for Elegant OTA - web controlled OTA update
 * 
 ***********************************************************************/

#include "main.h"

// VERSION #define goes here
#define VERSION "0.01"
#define PRG_NAME "ESP8266_MQTT_TEMP"
extern const String version = VERSION;
extern const String prgName = PRG_NAME;

void setup() {
  pinMode(LED_BUILT_IN_AUX, OUTPUT);
  digitalWrite(LED_BUILT_IN_AUX, 0);  // turn off LED

  Serial.begin(SERIAL_BAUD); // Start the Serial communication to send messages to the computer
  delay(1000);

  //while(!Serial) {} // Wait
  Serial.println("\n");
  Serial.print(prgName);
  Serial.print(" v");
  Serial.println(version);
  Serial.println("...initializing...");

  Serial.println("...Starting I2C...");
  Wire.begin(SDA, SCL);
  delay(1000);

  // Initialize and connect to WiFi
  Serial.println("...connecting WiFi...");
  WiFi_Init();  // connect to WiFi

  // Initialize Over the Air update handler
  OTA_Init();

  // Just to be sure we have a 404 response on unknown requests
  server.onNotFound(
      [](AsyncWebServerRequest *request) {
        // Send back a plain text message (can be made html if required)
        request->send(404, "text/plain", "404 - Page Not Found, oops!");
      });
  
  // Send back a web page (landing page)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Tell the system that we will be sending back html (not just plain text)
    AsyncResponseStream *response = request->beginResponseStream("text/html");

    // Format the html as a RAW literal so we don't have to escape all the quotes (and more)
    response->printf(R"(
      <html>
        <head>
          <title>Landing Page</title>
          <style>
            body {
              background-color: forestgreen;
              font-family: Arial, Sans-Serif;
            }
          </style>
        </head>
        <body>
          <h1>Landing Page</h1>
          <p>
            This is a descriptive paragraph for your landing page
          </p>
        </body>
      </html>
    )");

    // Send back all the text/html for a standard web page
    request->send(response);
  });

  // Starting Async OTA web server AFTER all the server.on requests registered
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("...Elegant OTA Ready...");
}

unsigned long previousTime = millis();
const unsigned long interval = 500;
int led = LED_BUILT_IN_AUX;

void loop() {
  ArduinoOTA.handle();
  unsigned long diff = millis() - previousTime;
  if(diff > interval) {
      digitalWrite(led, !digitalRead(led)); // Change the state of the LED
      previousTime += diff;
      //Serial.println("blink");
  }
  // yield to any other tasks waiting
  yield();
}