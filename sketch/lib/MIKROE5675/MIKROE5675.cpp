/**********************************************************************
 * @file MIKROE5675.cpp
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface to a MikroE 5675 module.
 * @version 0.1
 * @date 2023-02-08
 * @copyright Copyright (c) 2023
 */

#include <cstddef>
#include <Arduino.h>
#include <Wire.h>
#include "MIKROE5675.h"

MIKROE5675::MIKROE5675(MIKROE5675::tConfig config) {
  this->config = config;
} 

int MIKROE5675::reset() {
  int retval = -1;

  digitalWrite(this->config.gpioReset, 0);
  delay(100);
  digitalWrite(this->config.gpioReset, 1);
  delay(100);
  Wire.beginTransmission(this->config.address);
  Wire.write(MIKROE5675::PCA9538A_CONFIG_REGISTER);
  Wire.write(MIKROE5675::PCA9538A_CONFIG_DEFAULT);
  if ((retval = Wire.endTransmission()) == 0) {
    Wire.beginTransmission(this->config.address);
    Wire.write(MIKROE5675::PCA9538A_OUTPUT_REGISTER);
    Wire.write(0x00);
    retval = Wire.endTransmission();
  } else retval = (0 - retval);
  return(retval);
}

int MIKROE5675::getStatus() {
  int retval = -1;

  Wire.beginTransmission(this->config.address);
  Wire.write(MIKROE5675::PCA9538A_OUTPUT_REGISTER);
  if ((retval = Wire.endTransmission()) == 0) {
    Wire.requestFrom(this->config.address, 0x01);
    if (Wire.available()) {
      retval = (int) Wire.read();
    }
  } else retval = (0 - retval);
  return(retval);
}

int MIKROE5675::setStatus(uint8_t status) {
  Wire.beginTransmission(this->config.address);
  Wire.write(MIKROE5675::PCA9538A_OUTPUT_REGISTER);
  Wire.write(status);
  return((int) 0 - Wire.endTransmission());
}

void MIKROE5675::configureCallback(void (*callback)(int status), unsigned long callbackInterval) {
  this->callback = callback;
  this->callbackInterval = callbackInterval;
}
    
void MIKROE5675::callbackMaybe(bool force) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  int status;

  if (this->callback) {
    if (((this->callbackInterval) && (now > deadline)) || force) {
      if ((status = this->getStatus()) >= 0) {
        this->callback(status);
      }
      deadline = (now + this->callbackInterval);
    }
  }
}
