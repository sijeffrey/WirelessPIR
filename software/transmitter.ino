#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//#define LED_PIN 4
#define LED_PIN 8
RF24 radio(9, 10);
const byte rxAddr[6] = "00001";

void setup() {
  // Set up the radio
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  radio.stopListening();
  
  // Set up local indication
  pinMode(LED_PIN, OUTPUT);
  
  // wait 60 seconds for PIR to settle
  for (int i=0; i<60; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(950);
  }
  
  //Save Power by writing all Digital IO LOW - note that pins just need to be tied one way or another, do not damage devices!
  for (int i = 0; i < 20; i++) {
    if(i != 2)//just because the button is hooked up to digital pin 2
    pinMode(i, OUTPUT);
  }
  
  // Assign the interrupt PIN
  attachInterrupt(0, digitalInterrupt, RISING); //interrupt for waking up
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //Disable ADC
  ADCSRA &= ~(1 << 7); //Disable ADC - don't forget to flip back after waking up if using ADC in your application ADCSRA |= (1 << 7);
  
  //ENABLE SLEEP
  SMCR |= (1 << 2); //power down mode
  SMCR |= 1;//enable sleep

  //BOD DISABLE
  MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
  __asm__  __volatile__("sleep");//in line assembler to go to sleep
}

void digitalInterrupt() {
  //Wake up the ADC
  ADCSRA |= (1 << 7);
  
  //transmit data
  digitalWrite(LED_PIN, HIGH);
  char batt[7] = "0000mV";
  sprintf(batt,"%dmV", readVcc());
  radio.write(&batt, sizeof(batt));
  delay(50);
  digitalWrite(LED_PIN, LOW);
}

//Read Vcc voltage
long readVcc() {
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

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
