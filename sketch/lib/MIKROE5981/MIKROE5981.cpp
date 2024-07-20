/**********************************************************************
 * @file MIKROE5981.cpp
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface to a MikroE 5981 module.
 * @version 0.1
 * @date 2023-02-08
 * @copyright Copyright (c) 2023
 */

#include <cstddef>
#include <Arduino.h>
#include <SPI.h>
#include "MIKROE5981.h"

MIKROE5981::MIKROE5981(tPins pins, uint32_t speed, uint8_t order, uint8_t mode) {
  this->pins = pins;
  this->speed = speed;
  this->order = order;
  this->mode = mode;

  pinMode(this->pins.cs, OUTPUT);
  pinMode(this->pins.en, OUTPUT);
  pinMode(this->pins.rst, OUTPUT);
  this->deselect();
  this->enable();
  this->reset();

}

void MIKROE5981::reset() {
  digitalWrite(this->pins.rst, 0);
  delay(100);
  digitalWrite(this->pins.rst, 1);
  delay(100);
}

void MIKROE5981::enable() {
  digitalWrite(this->pins.en, 1);
}

void MIKROE5981::disable() {
  digitalWrite(this->pins.en, 0);
}

void MIKROE5981::select() {
  digitalWrite(this->pins.cs, 0);
}

void MIKROE5981::deselect() {
  digitalWrite(this->pins.cs, 1);
}
    
uint8_t MIKROE5981::read() {
  uint8_t status = 0;

  SPI.beginTransaction(SPISettings(this->speed, this->order, this->mode));
  this->select();
  status = SPI.transfer(0);
  this->deselect();
  SPI.endTransaction();
  return(status);
}

void MIKROE5981::configureCallback(void (*callback)(uint8_t status), unsigned long callbackInterval) {
  this->callback = callback;
  this->callbackInterval = callbackInterval;
}
    
void MIKROE5981::callbackMaybe(bool force) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();

  if (this->callback) {
    if (((this->callbackInterval) && (now > deadline)) || force) {
      this->callback(this->read());
      deadline = (now + this->callbackInterval);
    }
  }
}
