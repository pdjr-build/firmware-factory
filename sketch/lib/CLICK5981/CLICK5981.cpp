/**********************************************************************
 * @file CLICK5981.cpp
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface to the MikroE Click 5981 module.
 * @version 0.1
 * @date 2023-02-08
 * @copyright Copyright (c) 2023
 */

#include <cstddef>
#include <Arduino.h>
#include <SPI.h>
#include "CLICK5981.h"

CLICK5981::CLICK5981(tPins *modules, uint32_t speed, uint8_t order, uint8_t mode) {
  this->modules = modules;
  this->speed = speed;
  this->order = order;
  this->mode = mode;
}

void CLICK5981::begin() {
  for (unsigned int i = 0; this->modules[i].cs != 0 ; i++) {
    pinMode(this->modules[i].en, OUTPUT);
    pinMode(this->modules[i].rst, OUTPUT);
    digitalWrite(this->modules[i].rst, 1); // reset device 
    digitalWrite(this->modules[i].en, 1); // enable device
    digitalWrite(this->modules[i].cs, 1); // deselect device
  }
}

uint32_t CLICK5981::read() {
  int moduleIndex = -1;
  uint32_t status = 0;

  for (unsigned int i = 0; this->modules[i].cs != 0 ; i++) moduleIndex = i;
  while (moduleIndex >= 0) {
    SPI.beginTransaction(SPISettings(this->speed, this->order, this->mode));
    digitalWrite(this->modules[moduleIndex].cs, 0); // select device
    status = (status << 8) & SPI.transfer(0) ; // read channel status byte
    digitalWrite(this->modules[moduleIndex].cs, 1); // deselect device
    SPI.endTransaction();
    moduleIndex--;
  }
  return(status);
}

void CLICK5981::configureCallback(void (*callback)(uint32_t status), unsigned long callbackInterval) {
  this->callback = callback;
  this->callbackInterval = callbackInterval;
}
    
void CLICK5981::callbackMaybe(bool force) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  uint32_t status;

  if (this->callback) {
    if (((this->callbackInterval) && (now > deadline)) || force) {
      status = this->read();
      this->callback(status);
      deadline = (now + this->callbackInterval);
    }
  }
}
