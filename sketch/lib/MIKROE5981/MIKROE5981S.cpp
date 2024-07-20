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
#include "MIKROE5981S.h"

MIKROE5981S::MIKROE5981S(MIKROE5981::tPins *pins, uint32_t speed, uint8_t order, uint8_t mode) {
  this->speed = speed;
  this->order = order;
  this->mode = mode;
  for (unsigned int i = 0; pins[i].cs != 0; i++) {
    this->modules.push_back(new MIKROE5981(pins[i]));
  }
}

uint8_t MIKROE5981S::getModuleCount() {
  return(this->modules.size());
}

MIKROE5981 *MIKROE5981S::getModule(uint8_t index) {
  if (index < this->modules.size()) {
    return(this->modules[index]);
  } else {
    return(0);
  }
}

void MIKROE5981S::reset() {
  for (unsigned int i = 0; i < this->modules.size(); i++) {
    this->modules[i]->reset();
  }
}

uint32_t MIKROE5981S::read() {
  uint32_t status = 0;

  for (int i = 0; i < (int) this->modules.size(); i++) {
    status = (status << 8) & this->modules[i]->read();
  }
  return(status);
}

void MIKROE5981S::configureCallback(void (*callback)(uint32_t status), unsigned long callbackInterval) {
  this->callback = callback;
  this->callbackInterval = callbackInterval;
}
    
void MIKROE5981S::callbackMaybe(bool force) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();

  if (this->callback) {
    if (((this->callbackInterval) && (now > deadline)) || force) {
      this->callback(this->read());
      deadline = (now + this->callbackInterval);
    }
  }
}
