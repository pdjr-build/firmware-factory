/**********************************************************************
 * WindlassState.cpp - in memory cache of windlass run-time properties.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include "WindlassState.h"

WindlassState::WindlassState(unsigned char instance, int gpioStatusLed, int gpioUpRelay, int gpioDownRelay) {
  this->instance = instance;
  this->address = 0xFF;
  this->statusLedManager = NULL;
  this->stateLedManager = NULL;
  this->gpioStatusLed = gpioStatusLed;
  this->gpioUpLed = gpioUpLed;
  this->gpioDownLed = gpioDownLed;
  this->state = UNKNOWN;
}

void WindlassState::setLedManagers(LedManager *statusLedManager, LedManager *stateLedManager) {
  this->statusLedManager = statusLedManager;
  this->stateLedManager = stateLedManager;
  this->updateStatusLed();
}

void WindlassState::setInstance(unsigned char instance) {
  this->instance = instance;
  this->updateStatusLed();
}

void WindlassState::setAddress(unsigned char address) {
  this->address = address;
  this->updateStatusLed();
}

void WindlassState::setState(WindlassState::State state) {
  this->state = state;
  this->updateStateLed();
}

unsigned char WindlassState::getInstance() {
  return(this->instance);
}

unsigned char WindlassState::getAddress() {
  return(this->address);
}

bool WindlassState::isDisabled() {
  return(this->instance == WINDLASSSTATE_DISABLED_INSTANCE_VALUE);
}

bool WindlassState::isConfigured() {
  return((this->instance != 0xFF) && (this->instance != WINDLASSSTATE_DISABLED_INSTANCE_VALUE));
}

bool WindlassState::isReady() {
  return(this->isConfigured() && (this->address != 0xFF));
}

void WindlassState::updateStatusLed() {
  if (this->statusLedManager != NULL) {
    if (this->instance == 0xFF) { // Instance not set, so two flashes
      this->statusLedManager->operate(this->gpioStatusLed, 0, -2);
    } else {
      if (this->instance == WINDLASSSTATE_DISABLED_INSTANCE_VALUE) { // Disbled, so turn off
        this->statusLedManager->operate(this->gpioStatusLed, 0, 0);
      } else {
        if (this->address == 255) { // No bus address, so one flash
          this->statusLedManager->operate(this->gpioStatusLed, 0, -1);
        } else { // Ready to go, so LED on all the time
          this->statusLedManager->operate(this->gpioStatusLed, 1, 0);
        }
      }
    }
  }
}

void WindlassState::updateStateLed() {
  if (this->stateLedManager != NULL) {
    switch (this->state) {
      case DOCKED:
        this->stateLedManager->operate(this->gpioUpLed, 1);
        this->stateLedManager->operate(this->gpioDownLed, 0);
        break;
      case DEPLOYING:
        this->stateLedManager->operate(this->gpioUpLed, 0);
        this->stateLedManager->operate(this->gpioDownLed, 0, -1);
        break;
      case DEPLOYED:
        this->stateLedManager->operate(this->gpioUpLed, 0);
        this->stateLedManager->operate(this->gpioDownLed, 1);
        break;
      case RETRIEVING:
        this->stateLedManager->operate(this->gpioUpLed, 0, -1);
        this->stateLedManager->operate(this->gpioDownLed, 0);
        break;
      default:
        this->stateLedManager->operate(this->gpioUpLed, 0, -1);
        this->stateLedManager->operate(this->gpioDownLed, 0, -1);
        break;
    }
  }
}

