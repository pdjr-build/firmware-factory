/**********************************************************************
 * @file MIKROE5675S.cpp
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface to a MikroE 5675 module.
 * @version 0.1
 * @date 2023-02-08
 * @copyright Copyright (c) 2023
 */

#include <cstddef>
#include <Arduino.h>
#include <Wire.h>
#include "MIKROE5675S.h"

MIKROE5675S::MIKROE5675S(MIKROE5675::tConfig *configs) {
  for (unsigned int i = 0; configs[i].address != 0; i++) {
    this->modules.push_back(new MIKROE5675(configs[i]));
  }
}

int MIKROE5675S::getModuleCount() {
  return(this->modules.size());
}

int MIKROE5675S::reset() {
  int retval = 0;

  for (unsigned int i = 0; i < this->modules.size(); i++) {
    retval |= this->modules[i]->reset();
  }
  return(retval);
}

int MIKROE5675S::getStatus() {
  int retval = 0;
  int moduleStatus;

  for (unsigned int i = 0; i < this->modules.size(); i++) {
    moduleStatus = this->modules[i]->getStatus();
    if (moduleStatus >= 0) {
      retval = (retval << MIKROE5675::CHANNEL_COUNT) | moduleStatus;
    } else {
      return(0 - moduleStatus);
    }
  }
  return(retval);
}

int MIKROE5675S::setStatus(uint32_t status) {
  int retval = 0;
  int moduleStatus;

  for (unsigned int i = 0; i < this->modules.size(); i++) {
    moduleStatus = (status >> MIKROE5675::CHANNEL_COUNT);
    retval = this->modules[i]->setStatus(moduleStatus);
    if (retval < 0) return(retval);
  }
  return(retval);
}

void MIKROE5675S::configureCallback(void (*callback)(int status), unsigned long callbackInterval) {
  this->callback = callback;
  this->callbackInterval = callbackInterval;
}
    
void MIKROE5675S::callbackMaybe(bool force) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();

  if (this->callback) {
    if (((this->callbackInterval) && (now > deadline)) || force) {
      this->callback(this->getStatus());
      deadline = (now + this->callbackInterval);
    }
  }
}
