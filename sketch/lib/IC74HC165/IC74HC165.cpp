/**********************************************************************
 * IC74HC165.cpp - ADT for accessing a 74HC165 PISO buffer.
 * 2022 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <Arduino.h>
#include <IC74HC165.h>

IC74HC165::IC74HC165(uint8_t gpioData, uint8_t gpioLatch, uint8_t gpioClock) {
  this->gpioData = gpioData;
  this->gpioLatch = gpioLatch;
  this->gpioClock = gpioClock;
  this->lastsample = 0;
}

void IC74HC165::begin() {
  pinMode(this->gpioData, INPUT);
  pinMode(this->gpioLatch, OUTPUT);
  pinMode(this->gpioClock, OUTPUT);
}

uint8_t IC74HC165::readByte() {
  uint8_t retval = 0;
  digitalWrite(this->gpioClock, HIGH);
  digitalWrite(this->gpioLatch, HIGH);
  retval = shiftIn(this->gpioData, this->gpioClock, MSBFIRST);
  digitalWrite(this->gpioLatch. LOW);
  return(retval);
}

int IC74HC165::readBit(int bit) {
  return(((bit >= 0) && (bit <= 7))?bitRead(this->readByte(), bit):-1);
}
