#ifndef CREDENTIALS_h
#define CREDENTIALS_h

// local AP
const char* ssid     = " ";
const char* password = " ";

// static IP
const IPAddress ip(192, 168, 2, 110);
// const IPAddress dns(8, 8, 8, 8);
const IPAddress gateway(192, 168, 2, 1);
const IPAddress subnet(255, 255, 255, 0);

// MQTT Server
const char* MQTT_SERVER    = "raspberrypi.local";
const uint16_t MQTT_PORT   = 1883;
const char* MQTT_ID        = "ESP8266-DTH1";
const char* MQTT_USER      = "admin";
const char* MQTT_PSWD      = "admin";

// MQTT Topic
const char* MQTT_TOPIC_OUT = "/SOLAR/msg";
const char* MQTT_TOPIC_IN  = "/SOLAR/cmd";

#endif
