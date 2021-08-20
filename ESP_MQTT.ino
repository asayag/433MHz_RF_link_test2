/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "credentials.h"

#include <RH_ASK.h>
#include <SPI.h>
RH_ASK rf_driver(2000, 2, 0, 0); // RX,TX

WiFiClient espClient;
PubSubClient client(espClient);

#define BAUD 57600

#define MAX_MSG_SIZE 128
char msg[MAX_MSG_SIZE + 1];
byte msg_pos;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP); // Default is WIFI_NONE_SLEEP
  WiFi.setOutputPower(10); // 0~20.5dBm, Default max

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet); // For Static IP
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.write(payload[i]);
  }
  Serial.write('\n');
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_ID, MQTT_USER, MQTT_PSWD)) {
      Serial.println("connected");
      // Wait 5 seconds before retrying
      for (byte i = 0; i < 2; i++) delay(500); // delay(1000) may cause hang
      // Once connected, publish an announcement...
      // client.publish(MQTT_TOPIC_OUT, "hello world");
      // ... and resubscribe
      // client.subscribe(MQTT_TOPIC_IN);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for (byte i = 0; i < 10; i++) delay(500); // delay(5000) may cause hang
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// setup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() {
  Serial.begin(BAUD, SERIAL_8N1, SERIAL_FULL);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);


  // prepare GPIO2
  //pinMode(2, INPUT);
  // Initialize ASK Object
  rf_driver.init();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float getSupplyVoltage()
{
  int rawLevel = analogRead(A0);

  //float realVoltage = (float)rawLevel / 1000 / (10000. / (47000 + 10000));
  float realVoltage = (float)rawLevel / 1023 * 3.3;

  return realVoltage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

unsigned long lastMsg = 0;
void loop() {
  StaticJsonDocument < MAX_MSG_SIZE + 1 > doc; //256 is the RAM allocated to this document.
  // Set buffer to size of expected message
  uint8_t buf[MAX_MSG_SIZE + 1] = "";
  uint8_t buflen = sizeof(buf);

  int new_msg = 0;
  char mqtt_msg[MAX_MSG_SIZE + 1];
  int index, I, V, Vb;
  //~~~~~~
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //~~~~~~
  unsigned long now = millis();
  if (now - lastMsg > 30000) {
    Serial.println("I am Active ");
    // update last
    lastMsg = now;
  }
  //~~~~~~

  // Check if received packet is correct size
  if (rf_driver.recv(buf, &buflen)) {
    // Message received with valid checksum
    Serial.print("Received: ");
    Serial.println((char*)buf);
    Serial.println(sizeof(buf));
    memcpy(msg, buf, sizeof(buf));
    //    Serial.println(msg);
    //    Serial.println(msg[0]);
    //    Serial.println(msg[1]);
    //    Serial.println(msg[2]);
    //    Serial.println(msg[3]);
    //    delay(200);
    new_msg = 1;
  }


  // msg always start with "bat1"
  if ((msg[0] == 'b') && (msg[1] == 'a') && (msg[2] == 't') && (msg[3] == '1') && (new_msg == 1)) {
    Serial.println("msg");
    Serial.println(msg);

    int n = sscanf(msg, "bat1,#%d,V,%d,I,%d,Ve,%d", &index, &V, &I, &Vb);
    //    Serial.print(F("index="));
    //    Serial.print(index);
    //    Serial.print(F(", V="));
    //    Serial.print(V);
    //    Serial.print(F(", I="));
    //    Serial.println(I);

     memset(msg, 0, sizeof(msg)); //clear msg it is char array

    doc["Sensor"] = "bat1";
    doc["#"] = index;
    doc["V"] = V;
    doc["I"] = I;
    doc["bat"] = Vb;

    
    int b = serializeJson(doc, mqtt_msg); // Generate the minified JSON and send it to the msg char array
    Serial.print("Publish message: ");
    Serial.println(mqtt_msg);
    client.publish(MQTT_TOPIC_OUT, mqtt_msg);

    new_msg = 0;
  }
  else delay(7); // Wait for a complete minimal message


  //  while ((Serial.available() > 0) && (msg_pos < MAX_MSG_SIZE)) // Chek for availablity of data at Serial Port
  //  {
  //    msg[msg_pos++] = Serial.read(); // Reading Serial Data and saving in data variable
  //    delayMicroseconds(174U);        // 174µs/char @ 57600 Bauds -> bit_time = 1 / baud_rate  --> char * 10bit
  //    //delayMicroseconds(4160U);        //
  //    new_msg = 1;
  //  }
  //  //{"Sensor":"DTH2","Msg#":5,"data":[28.2,40.1]}
  //  msg[msg_pos++] = 0; // add Null byte at the end*/


  //  //  // msg min size 40
  //  //  // msg always start with "bat1"
  //  if ((msg[0] == 'b') && (msg[1] == 'a') && (msg[2] == 't') && (msg[2] == '1') && ( new_msg == 1)) {
  //    //char* msg = "bat1,#31,V,0,I,1340";
  //    int n = sscanf(msg, "bat1,#%d,V,%d,I,%d,Ve,%d", &index, &V, &I, &Vb);
  //    Serial.print(F("index="));
  //    Serial.print(index);
  //    Serial.print(F(", V="));
  //    Serial.print(V);
  //    Serial.print(F(", I="));
  //    Serial.println(I);
  //
  //    doc["Sensor"] = "bat1";
  //    doc["#"] = index;
  //    doc["V"] = V;
  //    doc["I"] = I;
  //    int b = serializeJson(doc, mqtt_msg); // Generate the minified JSON and send it to the msg char array
  //    Serial.print("Publish message: ");
  //    Serial.println(mqtt_msg);
  //    client.publish(MQTT_TOPIC_OUT, mqtt_msg);
  //    new_msg = 0;
  //  }
  //  else delay(7); // Wait for a complete minimal message

  //    char* msg1 = "bat1,#31,V,0,I,1340";
  //    int n = sscanf(msg1, "bat1,#%d,V,%d,I,%d", &index, &V, &I);
  //    Serial.print(F("index="));
  //    Serial.print(index);
  //    Serial.print(F(", V="));
  //    Serial.print(V);
  //    Serial.print(F(", I="));
  //    Serial.println(I);

  // msg min size 40
  // msg always start with "20;"
  //  if ((new_msg == 1) && (msg_pos > 10))
  //  {
  //    if (!client.connected()) reconnect();
  //    client.loop();
  //    Serial.print("Publish message: ");
  //    Serial.println(msg);
  //    client.publish(MQTT_TOPIC_OUT, msg);
  //    new_msg = 0;
  //  }
  //  else delay(7); // Wait for a complete minimal message

  delay(10); // To Enable some more Modem Sleep

}
