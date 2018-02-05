#include "avr/interrupt.h"

volatile unsigned long rc_timeStart1;
volatile unsigned long rc_timeStart2;
volatile int rc_value1;
volatile int rc_value2;
volatile byte rcPin1LastState = 0;
volatile byte rcPin2LastState = 0;

int headlightStrength = 0;
int throttlePercentage = 0;
const byte pwmPin1 = 0;
const byte pwmPin2 = 1;
const byte rcPin1 = 2;
const byte rcPin2 = 3;
const int brakeLightTrigger = 40;
const bool stepping = true;

void setup() {
  pinMode(rcPin1, INPUT);
  pinMode(rcPin2, INPUT);
  pinMode(pwmPin1, OUTPUT);
  pinMode(pwmPin2, OUTPUT);

  GIMSK = 0b00100000; //turn on pin change interrupts
  PCMSK = 0b00001100; //turn on interrupts on pin PB2 and PB3 (physical pins 2 & 3)
  sei();

  TCCR1 = 0b00001110; //set the clock select bits of the Timer1 control register to make it run at ~980Hz so that our PWM output is close to 1KHz
}

void loop() {
  headlightStrength = map(rc_value1, 1000, 2000, 0, 255); //assume the receiver is outputting exactly in spec between 1000 and 2000; it's probably not but should be close enough
  headlightStrength = constrain(headlightStrength, 0, 255);
  if(stepping == true) {
    headlightStrength /= 10; //super naive stepping
    headlightStrength *= 10;
  }
  analogWrite(pwmPin1, headlightStrength);
  throttlePercentage = map(rc_value2, 1000, 2000, 0, 100);
  if(throttlePercentage > brakeLightTrigger) {
    analogWrite(pwmPin2, 127); //half strength brake light, make it a taillight
  } else {
    analogWrite(pwmPin2, 255); //full strength brake light for maximum stopping redness
  }
}

ISR(PCINT0_vect)
{
  if(digitalRead(rcPin1) == HIGH) {
    if(rcPin1LastState == 0) {
      rc_timeStart1 = micros();
      rcPin1LastState = 1;
    }
  } else {
    if(rcPin1LastState == 1) {
      rc_value1 = micros() - rc_timeStart1;
      rcPin1LastState = 0;
    }
  }
  if(digitalRead(rcPin2) == HIGH) {
    if(rcPin2LastState == 0) {
      rc_timeStart2 = micros();
      rcPin2LastState = 1;
    }
  } else {
    if(rcPin2LastState == 1) {
      rc_value2 = micros() - rc_timeStart2;
      rcPin2LastState = 0;
    }
  }
}
