#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);
int led = 4;
int relay = 3;
int count = 0;
const long interval = 60000; // 60 seconds
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

const byte rxAddr[6] = "00001";

void setup()
{
  while (!Serial);
  Serial.begin(9600);
  
  radio.begin();
  radio.openReadingPipe(0, rxAddr);
  
  radio.startListening();
  
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
}

void loop()
{
  unsigned long currentMillis = millis();
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    
    //Serial.println(text);
    if (strstr(text,"mV") != 0)
    {
      digitalWrite(led, HIGH);
      long batteryVoltage = strtol(text, (char **)NULL, 10);
      digitalWrite(relay, LOW);
      //Serial.println(text);
      //Warn if battery low by double flashing lamp
      Serial.println(batteryVoltage);
      if (batteryVoltage < 2700)
      {
        delay(500);
        digitalWrite(relay, HIGH);
        delay(500);
        digitalWrite(relay, LOW);
      }
      previousMillis = currentMillis; // reset the difference to zero
      count = 0;
    } 
  }
  count++;
  if (count > 10000)
  {
    //Serial.println("none");
    digitalWrite(led, LOW);
    count = 10001; //stops int overrun
  }
  if (currentMillis - previousMillis >= interval) {
    // reset the relay once the difference is equal or greater than interval
    digitalWrite(relay, HIGH);
 }
}
