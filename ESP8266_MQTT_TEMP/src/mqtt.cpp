#include <Arduino.h>
#include "mqtt.h"

extern Config config;  // declare the external configuration struct
extern Status status;  // declare the external status struct

/*-------------------------------------------------------------------------
 * Function to initialize and connect to an MQTT broker defined in the
 * configuration struct.
 * Notes: use the following default values from the PubSubClient library:
 *        MQTT_MAX_PACKET_SIZE = 128 bytes
 *        MQTT_KEEPALIVE = 15 seconds
 *        MQTT_VERSION = MQTT 3.1.1
 *        MQTT_MAX_TRANSFER_SIZE = undefined (complete packet passed)
 *        MQTT_SOCKET_TIMEOUT = 15 seconds
 *-------------------------------------------------------------------------*/
bool connectMqtt() {
  bool mqttConnectedFlag = false;
  // set the MQTT server and port
  mqttClient.setServer(config.mqttServer, config.mqttPort);

  // set the MQTT message received callback function
  mqttClient.setCallback(mqttCallback);

  // initialize the MQTT topics for this device
  mqttTopicInit();

  // now set up the last will topic with
  // QoS = 1, willRetain = true, cleanSession = true

  // create the last will message
  sprintf(willMessage, "{\"%s\":{\"wifi\":\"Offline\"}}", status.host);

  mqttConnectedFlag = mqttClient.connect(status.host,
                      config.mqttUser,
                      config.mqttPW,
                      willTopic, willQoS, willRetain,
                      willMessage, cleanSession);
  // now report on connection status
  if (mqttConnectedFlag) {
    Serial.println("...MQTT broker connected...");
  }
  else {
    Serial.print("...ERROR: MQTT connect failed - ");
    mqttState();
  }

  return mqttConnectedFlag;

}

/*-------------------------------------------------------------------------
 * Function to handle messages received from subscribed MQTT topics.
 *-------------------------------------------------------------------------*/
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  strncpy(rcvTopic, topic, sizeof(rcvTopic)); // save the topic
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // print the payload and save to rcvMsg
  unsigned int i = 0;
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    rcvMsg[i] = (char)payload[i];
  }
  rcvMsg[i] = '\0'; // terminate the message with a null
  Serial.println();

  newMsgFlag = true;

  return;
}

/*-------------------------------------------------------------------------
 * Function to initialize the topic char arrays for this device
 *-------------------------------------------------------------------------*/
void mqttTopicInit() {
  char temp[30];

  // First create the initial topic path with topicPreamble + device name
  strncpy(temp, topicPreamble, sizeof(temp));
  strncat(temp, status.host, sizeof(temp));

  // Now initialize the will topic
  strncpy(willTopic, temp, sizeof(willTopic));
  strncat(willTopic, will, sizeof(willTopic));

  // Now initialize the input topic that will be monitored
  strncpy(inTopic, temp, sizeof(inTopic));
  strncat(inTopic, cmd, sizeof(inTopic));

  // Now initialize the output topic that will represent the device status
  strncpy(outTopic, temp, sizeof(outTopic));
  strncat(outTopic, statusTopic, sizeof(outTopic));

}

/*-------------------------------------------------------------------------
 * Function to print the MQTT connection state after an error
 *-------------------------------------------------------------------------*/
int mqttState() {
  // now report why it failed
  int state = mqttClient.state();

  switch (state) {
    case MQTT_CONNECTED : {
      Serial.println("MQTT_CONNECTION_TIMEOUT");
      break;
    }
    case MQTT_CONNECTION_TIMEOUT : {
      Serial.println("MQTT_CONNECTION_TIMEOUT");
      break;
    }
    case MQTT_CONNECTION_LOST : {
      Serial.println("MQTT_CONNECTION_LOST");
      break;
    }
    case MQTT_CONNECT_FAILED : {
      Serial.println("MQTT_CONNECT_FAILED");
      break;
    }
    case MQTT_DISCONNECTED : {
      Serial.println("MQTT_DISCONNECTED");
      break;
    }
    case MQTT_CONNECT_BAD_PROTOCOL : {
      Serial.println("MQTT_CONNECT_BAD_PROTOCOL");
      break;
    }
    case MQTT_CONNECT_BAD_CLIENT_ID : {
      Serial.println("MQTT_CONNECT_BAD_CLIENT_ID");
      break;
    }
    case MQTT_CONNECT_UNAVAILABLE : {
      Serial.println("MQTT_CONNECT_UNAVAILABLE");
      break;
    }
    case MQTT_CONNECT_BAD_CREDENTIALS : {
      Serial.println("MQTT_CONNECT_BAD_CREDENTIALS");
      break;
    }
    case MQTT_CONNECT_UNAUTHORIZED : {
      Serial.println("MQTT_CONNECT_UNAUTHORIZED");
      break;
    }
    default:
      Serial.print("Invalid MQTT status code: ");
      Serial.println(state);
  }
  return state;
}
