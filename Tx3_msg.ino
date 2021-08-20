// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>
// Include dependant SPI Library
#include <SPI.h>
#include "LowPower.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Create Amplitude Shift Keying Object
//RH_ASK rf_driver;
//RH_ASK rf_driver(2000, 11, 12, 0);  //Leonardo  // RX,TX
RH_ASK rf_driver(2000, 11, 15, 0);  //pro micro  // RX,TX
#define Pin_CSens1 A3 // current sensor1
#define Pin_VSens1 A1 // output voltage port1
#define Pin_Vext A2 // output voltage port1

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup()
{
  for (int pin = 0; pin < 11; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(A0, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB
//  }
  // Initialize ASK Object
  rf_driver.init();
  Serial.println("Start");
  delay(10000);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (MAX_MSG_SIZE + 1)
int value = 0;
const char* outTopic = "bat1";

void loop()
{
  char outMessage[100] = "";

  unsigned long now = millis();
  // if (now - lastMsg > 5000) {
  int V = normVolt(analogRead(Pin_VSens1));
  int Vext = normVolt(analogRead(Pin_Vext));
  int I = normVolt(analogRead(Pin_CSens1));

  //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  sprintf(outMessage, "%s,#%d,V,%d,I,%d,Ve,%d", outTopic, value, V, I,Vext);
  Serial.print("Publish message: ");
  Serial.println(outMessage);
  Serial.println(strlen(outMessage));

  delay(200);
  rf_driver.send((uint8_t *)outMessage, strlen(outMessage));
  rf_driver.waitPacketSent();
  ++value;

  // update last
  lastMsg = now;

  // }

  // Enter power down state for 8 s with ADC and BOD module disabled
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  delay(2000);
}


// ----------------------------------------------------------------
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(4); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;
}
// ----------------------------------------------------------------
int normVolt(int reading) {
  float vcc = readVcc() / 1000.0;
  //float voltage = (float(reading) / 1023.0) * vcc;
  int voltage = int(float(reading) * vcc);
  return voltage;
}


//int id1, id2, id3;
//int pos1, pos2, pos3;
//char* buf = "1:90&2:80&3:180";
//int n = sscanf(buf, "%d:%d&%d:%d&%d:%d", &id1, &pos1, &id2, &pos2, &id3, &pos3);
//Serial.print(F("n="));
//Serial.println(n);
//Serial.print(F("id1="));
//Serial.print(id1);
//Serial.print(F(", pos1="));
//Serial.println(pos1);
//Serial.print(F("id2="));
//Serial.print(id2);
//Serial.print(F(", pos2="));
//Serial.println(pos2);
//Serial.print(F("id3="));
//Serial.print(id3);
//Serial.print(F(", pos3="));
//Serial.println(pos3);
//saleae
