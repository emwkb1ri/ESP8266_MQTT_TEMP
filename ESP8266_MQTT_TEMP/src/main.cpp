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
 * 1.00 - Clean compiled version with several changes for the Platform
 *        IO / Visual Studio Code environment
 * 
 ***********************************************************************/

#include "main.h"

// VERSION #define goes here
#define VERSION "1.00"
#define PRG_NAME "ESP8266_MQTT_TEMP"
extern const char version[] = VERSION;
extern const char prgName[] = PRG_NAME;

extern Config config;  // declare the external configuration struct
extern Status status;  // declare the external status struct

void setup() {
  pinMode(LED_BUILT_IN_AUX, OUTPUT);
  digitalWrite(LED_BUILT_IN_AUX, 0);  // turn off LED

  runTimer = millis();
  // use the following to keep ESP8266 running after wake-up
  pinMode(GPIO0, OUTPUT);     // configure GPIO0 as output pin
  //pinMode(GPIO2, OUTPUT);     // configures GPIO2 as output pin
  //digitalWrite(GPIO0, HIGH);  // keeps CH_PD set high - chip enabled
  //digitalWrite(GPIO2, LOW);   // initialize GPIO2 to LOW

  // setup GPIO14 to bypass deep sleep with a pulldown
  pinMode(GPIO14, INPUT_PULLUP);   // set GPIO14 as input with pullup

  Serial.begin(SERIAL_BAUD); // Start the Serial communication to send messages to the computer
  delay(1000);

  // Get the reason for the latest chip reset
  Serial.print("\r\n*** ");
  String reason = ESP.getResetReason();
  Serial.print(reason);
  Serial.println(" ***\r\n");

  switch (reason.charAt(0)) {
    // Deep Sleep Wake
    case 'D': {
      status.runTime = getSavedRunTime();
      Serial.printf("\r\n++++ Deep - runTime= %lu ++++\r\n", status.runTime);
      break;
    }
    // Power On or External Reset
    case 'E':
    case 'P': {
      // reset the msg count in RTC memory
      status.msgCount = 0;
      ESP.rtcUserMemoryWrite(4, &status.msgCount, sizeof(status.msgCount));
      status.runTime = 0;
      updateRunTime();
      Serial.printf("\r\n++++ Power - runTime= %lu ++++\r\n", status.runTime);
      break;
    }
    case 'S': {
      status.runTime = getSavedRunTime();
      Serial.printf("\r\n++++ Restart - runTime= %lu ++++\r\n", status.runTime);
      break;
    }
  }
  sleep = digitalRead(GPIO14);   // true - deep sleep, false - no sleep
  if (!sleep) {
    Serial.println("***SLEEP DISABLED***");

    //+++++++++++++++++
    // ESP8266 chip/system info
    Serial.println("\r\n########");
    Serial.printf("ChipID: %x \r\n", ESP.getChipId());
    Serial.printf("Core Version: ");
    Serial.println(ESP.getCoreVersion());
    Serial.printf("SDK Version: %s\r\n", ESP.getSdkVersion());
    Serial.printf("CPU Freq: %i MHz\r\n", ESP.getCpuFreqMHz());
    Serial.printf("Sketch size: %i bytes\r\n", ESP.getSketchSize());
    Serial.printf("Free Space: %i bytes\r\n", ESP.getFreeSketchSpace());
    Serial.printf("Flash ChipID: %x\r\n", ESP.getFlashChipId());
    //Serial.printf("Flash Chip Size: %i bytes\r\n", getFlashChipSize());
    //Serial.printf("Flash Chip Real: %i bytes\r\n", getFlashChipRealSize());
    //Serial.printf("Flash Chip Speed: %i Hz\r\n", getFlashChipSpeed());
    Serial.println("########\r\n");
  }

  // now print the current program information
  Serial.println("\n");
  Serial.print(prgName);
  Serial.print(" v");
  Serial.println(version);
  Serial.println("...initializing...");

  Serial.println("...Starting I2C...");
  Wire.begin(SDA, SCL);
  delay(1000);

  // initialize the One Wire temperature sensor interface
  oneWireInit();

  // Initialize and connect to WiFi
  Serial.println("...connecting WiFi...");
  WiFi_Init();  // connect to WiFi

  // Initialize Over the Air update handler
  OTA_Init();

  // Initialize web based OTA update handler
  elegantOTA_Init();

  //+++++++++++++++++++++++++++++
  //Setup the MQTT functions
  // connect to broker passing pointers to the PubSubClinet and Config struct
  if (!connectMqtt()) {
    updateRunTime();
    Serial.printf("\r\n++++ MQTT 1 - runTime= %lu ++++\r\n", status.runTime);
    ESP.restart();  // restart if not connected
  }

  if (!mqttClient.subscribe(inTopic, QOS_1)) {  // subscribe to the input topic
    Serial.println("ERROR: Subscribe to '/cmd' failed - Resarting");
    updateRunTime();
    Serial.printf("\r\n++++ MQTT 2 - runTime= %lu ++++\r\n", status.runTime);
    ESP.restart();
  };
  Serial.println("MQTT subscribed to '/cmd'");
}

unsigned long previousTime = millis();
const unsigned long interval = 500;
int led = LED_BUILT_IN_AUX;
bool otaInProgress = false;

//+++++++++++++++++++++++++++++++++
// the main execution loop
void loop() {
  // initialize to force operations in first pass through loop
  unsigned long statusTimer = millis() + STATUS_INTERVAL; // status timer
  unsigned long tempTimer = millis() + TEMP_INTERVAL;   // temp interval timer
  //unsigned long tempTimer = millis();   // temp interval timer

  //++++++++++++++++
  // loop forever
  while(1) {
    // check sleep disable input pin
    sleep = digitalRead(GPIO14);   // true - deep sleep, false - no sleep
    delay(1);
    // check for OTA updates
    ArduinoOTA.handle();
    delay(1);

    if(!otaInProgress) {
      // process MQTT incoming messages by running PubSubClient.loop()
      if (!mqttClient.loop()) {
        Serial.print("ERROR: Connection lost");
        Serial.print("MQTT Connection State= ");
        Serial.println(mqttState());
        ESP.restart();
      }

      //+++++++++++++++++++++++++++++++++
      // handle any new messages received
      if (newMsgFlag) {
        if (strcmp(rcvTopic, inTopic) == 0) {
          // this is a valid inTopic message
          // now act on the message if it is "ON"
          if (strcmp(rcvMsg, "ON") == 0) {
            digitalWrite(RELAY, HIGH);
            strncpy(status.relay, "ON", sizeof(status.relay));
            Serial.println("RELAY ON");
          }
          // now act on the message if it is "OFF"
          else if (strcmp(rcvMsg, "OFF") == 0) {
            digitalWrite(RELAY, LOW);
            strncpy(status.relay, "OFF", sizeof(status.relay));
            Serial.println("RELAY OFF");
          }
          // now act on the message if it is "TOGGLE"
          else if (strcmp(rcvMsg, "TOGGLE") == 0) {
            if(digitalRead(RELAY)) {
              digitalWrite(RELAY, LOW);
              strncpy(status.relay, "OFF", sizeof(status.relay));
              Serial.println("RELAY OFF");
            }
            else {
              digitalWrite(RELAY, HIGH);
              strncpy(status.relay, "ON", sizeof(status.relay));
              Serial.println("RELAY ON");
            }
          }
        }

        // read the current pin state
        if (digitalRead(RELAY)) {
            strncpy(status.relay, "ON", sizeof(status.relay));
        }
        else {
            strncpy(status.relay, "OFF", sizeof(status.relay));
        }

        // publish the MQTT status message
        publishMsg1(outMsg);
        newMsgFlag = false;           // reset newMsgFlag
        //statusTimer = millis();       // reset the status timer
      }

      //+++++++++++++++++++++++++++++++++++
      //Temperature sensor execution block
      // call sensors.requestTemperatures() to issue a global temperature
      // request to all devices on the bus
      if (millis() - tempTimer > TEMP_INTERVAL) {
        tempTimer = millis();       // reset the timer
        //Serial.print("Requesting temperatures...");
        sensors.requestTemperatures(); // Send the command to get temperatures
        //Serial.println("DONE");
        // It responds almost immediately. Let's print out the data

        // Use a simple function to print and return sensor temperature
        for (int i = 0; i < numDevices; i++) {
          Serial.printf("Sensor %i - ", i);
          status.DegC[i] = printTemperature(tempSensor[i]);
          status.DegF[i] = DallasTemperature::toFahrenheit(status.DegC[i]);
        }
        //status.vcc = ((float)ESP.getVcc()/1024);
        int a0 = analogRead(A0);
        status.vcc = (float)a0 * volts_per_step;
        //status.vcc = analogRead(A0);
        Serial.printf("Vcc = %.2f - %i\r\n", status.vcc, a0);
      } // end temperature sensor execution block

      //+++++++++++++++++++++++++++++++
      // if status timer expires - publish the current status
      if (millis() - statusTimer > STATUS_INTERVAL) {
        statusTimer = millis();       // reset the timer

        if (mqttClient.connected()) {
          strncpy(status.wifi, "Online", sizeof(status.wifi));
        }
        else {
          strncpy(status.wifi, "Offline", sizeof(status.wifi));
        }

        // read the current pin state
        if (digitalRead(RELAY)) {
          strncpy(status.relay, "ON", sizeof(status.relay));
        }
        else {
            strncpy(status.relay, "OFF", sizeof(status.relay));
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++
        // print to serial and publish the status messages
        // publish the MQTT messages
        publishMsg1(outMsg);

        publishTemps(outMsg, numDevices);

      } // end publish status execution block
    } // !otaInProgress execution block

    //++++++++++++++++++++++++++++++++++++++++++++
    // enter deep sleep here unless sleep is false
    if(sleep) {
      wifiClient.flush();   // ensure all data has been sent before sleep
      Serial.println("*** Entering Deep Sleep ***");
      delay(5000);          // display temps for 5 seconds on OLED
      updateRunTime();
      Serial.printf("Run Time: %lu\r\n", status.runTime);
      ESP.deepSleep(SLEEP_TIME_TEN_MINUTES);  // set deep sleep time
      //ESP.deepSleep(SLEEP_TIME_SIXTY_SECONDS);  // set deep sleep time
    }
  } // OTA while loop block

  // yield to any other tasks waiting
  yield();
} // end main loop


/*-------------------------------------------------------------------------
 * Function to print the temperature for a device and return sensor temp
 *-------------------------------------------------------------------------*/
 float printTemperature(DeviceAddress deviceAddress) {
   // method 1 - slower
   //Serial.print("Temp C: ");
   //Serial.print(sensors.getTempC(deviceAddress));
   //Serial.print(" Temp F: ");
   //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

   // method 2 - faster
   float tempC = sensors.getTempC(deviceAddress);
   if(tempC == DEVICE_DISCONNECTED_C)
   {
     Serial.println("Error: Could not read temperature data");
     tempC = 0;
     return tempC;
   }

   Serial.print("Temp C: ");
   Serial.print(tempC);
   Serial.print(" Temp F: ");
   // Convert tempC to Fahrenheit
   float tempF = DallasTemperature::toFahrenheit(tempC);
   Serial.println(tempF);

   return tempC;
 }

 /**********************************************************
  * Function to print a device address
  **********************************************************/
 void printAddress(DeviceAddress deviceAddress) {
   for (uint8_t i = 0; i < 8; i++)
   {
     if (deviceAddress[i] < 16) Serial.print("0");
     Serial.print(deviceAddress[i], HEX);
   }
 }

/*-------------------------------------------------------------------------
 * Function to publish an MQTT message to outTopic
 *-------------------------------------------------------------------------*/
void publish(char* topic, char* msg) {
  // publish the MQTT message, else print error message and restart the ESP01
  if (!mqttClient.publish(topic, msg)) {
    Serial.printf("ERROR: failed to send '%s' message\n", msg);
    Serial.print("MQTT Connection State= ");
    Serial.println(mqttState());
    // if connected - disonnect if we got here
    if (mqttClient.connected()) mqttClient.disconnect();
    updateRunTime();
    Serial.printf("\r\n++++ MQTT 3 - runTime= %lu ++++\r\n", status.runTime);
    ESP.restart();
  }
}

/*------------------------------------------------------------------------
 * Function to assemble and publish the MQTT status message
 *------------------------------------------------------------------------*/
void publishMsg1(char msg[]) {

  // get the saved message count from RTC memory offset 4
  ESP.rtcUserMemoryRead(4, &status.msgCount, sizeof(status.msgCount));
  status.msgCount++;
  ESP.rtcUserMemoryWrite(4, &status.msgCount, sizeof(status.msgCount));

  /*sprintf(msg,
          "{\"%s\":{\"pgm\":\"%s\",\"version\":\"%s\",\"msg\":\"%u\",\"wifi\":\"%s\",\"rssi\":\"%i\",\"relay\":\"%s\"}}",
        status.host, prgName, version, status.msgCount, status.wifi, status.rssi, status.relay);
  */
  sprintf(msg,
          "{\"%s\":{\"version\":\"%s\",\"msg\":\"%u\",\"wifi\":\"%s\",\"rssi\":\"%i\",\"relay\":\"%s\"}}",
        status.host, version, status.msgCount, status.wifi, status.rssi, status.relay);
  // now publish it
  Serial.printf("[%s] %s\n", outTopic, outMsg);
  publish(outTopic, outMsg);    // publish the current status
  return;
}

/*------------------------------------------------------------------------
 * Function to assemble and publish the MQTT temp sensor messages
 *------------------------------------------------------------------------*/
void publishTemps(char msg[], int devices) {

  for (int i = 0; i < devices; i++) {
    //updateRunTime();
    // assemble temp sensor MQTT messages
    sprintf(msg,"{\"%s\":{\"Deg%iC\":\"%.2f\",\"Deg%iF\":\"%.2f\",\"vcc\":\"%.2f\",\"run\":\"%lu\"}}",
          status.host, i, status.DegC[i], i, status.DegF[i], status.vcc, status.runTime);
    // now publish it
    Serial.printf("[%s] %s\n", outTopic, outMsg);
    publish(outTopic, outMsg);    // publish the current pin state
    status.runTime = 0;           // reset after publishing last saved value
  }
  return;
}


/*-----------------------------------------------------------------------
 * Function to update the runTime variable stored in RTC memory
 * - adds current millis() value to runTime and saves to RTC memory
 * - note that unsigned long type in ESP8266/ESP32 is 32 bits
 *-----------------------------------------------------------------------*/
void updateRunTime() {
  //uint32_t upper;
  uint32_t lower;
  status.runTime += millis();
  Serial.printf("\r\n++++ Update - runTime= %lu ++++\r\n", status.runTime);
  lower = (status.runTime & 0xFFFFFFFF);
  //upper = ((status.runTime >> 32) & 0xFFFFFFFF);
  //Serial.printf("\r\n++++ Upper - runTime= %i ++++\r\n", upper);
  //Serial.printf("\r\n++++ Lower - runTime= %i ++++\r\n", lower);
  //ESP.rtcUserMemoryWrite(8, &upper, sizeof(upper));
  ESP.rtcUserMemoryWrite(12, &lower, sizeof(lower));
  return;
}

/*-----------------------------------------------------------------------
 * Function to read the runTime variable stored in RTC memory
 * - returns unsigned long milliseconds stored in 8 bytes of RTC memory
 * - note that unsigned long type in ESP8266/ESP32 is 32 bits
 *-----------------------------------------------------------------------*/
unsigned long getSavedRunTime() {
  //uint32_t upper;
  uint32_t lower;
  unsigned long time;
  //ESP.rtcUserMemoryRead(8, &upper, sizeof(upper));
  ESP.rtcUserMemoryRead(12, &lower, sizeof(lower));
  //time = (unsigned long)upper;  // assign upper data bits
  //time = time << 32;  // now shift upper data bits to high 32 bits
  //time = time | lower;  // now OR the lower data bits 
  time = lower;           // it's only a 32 bit number in ESP8266
  Serial.printf("\r\n++++ Read RTC - runTime= %lu ++++\r\n", time);
  return time;
}


//+++++++++++++++++++++++++++++++++++
// Initialize oneWire bus and DS18B20
// locate devices on the bus
void oneWireInit() {
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  numDevices = sensors.getDeviceCount();
  // limit support to NUM_DEVICES devices - so data fits in 256 byte MQTT
  // packet
  if(numDevices > MAX_DEVICES) {
    Serial.printf("More than %i devices reported - count set to %i/r/n",        MAX_DEVICES, MAX_DEVICES);
    numDevices = MAX_DEVICES;
  }

  Serial.print(numDevices, DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Assign address manually. The addresses below will need to be changed
  // to valid device addresses on your bus. Device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  // Note that you will need to use your specific address here
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };

  // Method 1:
  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  for (int i = 0; i < numDevices; i++) {
    if (!sensors.getAddress(tempSensor[i], i)) Serial.printf("Unable to find address for Device %i \r\n", i);
    //if (!sensors.getAddress(tempSensor1, 1)) Serial.println("Unable to find address for Device 1");
  }

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");

  // print the addresses of devices and set resolution
  for (int i = 0; i < numDevices; i++) {
    // print the address we found on the bus
    Serial.printf("Device %i Address: ", i);
    printAddress(tempSensor[i]);
    Serial.println();

    // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
    sensors.setResolution(tempSensor[i], 9);

    Serial.printf("Device %i Resolution: ", i);
    Serial.print(sensors.getResolution(tempSensor[i]), DEC);
    Serial.println();
  }
}

//++++++++++++++++++++++++++++++++++++
// Initialize the Elegant OTA web based Over the Air update functions
void elegantOTA_Init() {
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