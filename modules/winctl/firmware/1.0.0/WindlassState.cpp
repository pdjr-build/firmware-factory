/**********************************************************************
 * WindlassState.cpp - in memory cache of windlass run-time properties.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <Arduino.h>
#include <WindlassState.h>

WindlassState::WindlassState() {
  this->instance = 0xFF;
  this->address = 0xFF;
  this->state = UNKNOWN;

  this->programmeSwitchGPIO = -1;
  this->upSwitchGPIO = -1;
  this->downSwitchGPIO = -1;
  this->statusLedGPIO = -1;
  this->upLedGPIO = -1;
  this->downLedGPIO = -1;
  this->instanceStorageAddress = instanceStorageAddress;

  this->pDebouncer = NULL;
  this->pStatusLedManager = NULL;
  this->pStateLedManager = NULL;
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
  if (this->pStatusLedManager != NULL) {
    if (this->instance == 0xFF) { // Instance not set, so two flashes
      this->pStatusLedManager->operate(this->statusLedGPIO, 0, -2);
    } else {
      if (this->instance == WINDLASSSTATE_DISABLED_INSTANCE_VALUE) { // Disbled, so turn off
        this->pStatusLedManager->operate(this->statusLedGPIO, 0, 0);
      } else {
        if (this->address == 255) { // No bus address, so one flash
          this->pStatusLedManager->operate(this->statusLedGPIO, 0, -1);
        } else { // Ready to go, so LED on all the time
          this->pStatusLedManager->operate(this->statusLedGPIO, 1, 0);
        }
      }
    }
  }
}

void WindlassState::updateStateLed() {
  if (this->pStateLedManager != NULL) {
    switch (this->state) {
      case DOCKED:
        this->pStateLedManager->operate(this->upLedGPIO, 1);
        this->pStateLedManager->operate(this->downLedGPIO, 0);
        break;
      case DEPLOYING:
        this->pStateLedManager->operate(this->upLedGPIO, 0);
        this->pStateLedManager->operate(this->downLedGPIO, 0, -1);
        break;
      case DEPLOYED:
        this->pStateLedManager->operate(this->upLedGPIO, 0);
        this->pStateLedManager->operate(this->downLedGPIO, 1);
        break;
      case RETRIEVING:
        this->pStateLedManager->operate(this->upLedGPIO, 0, -1);
        this->pStateLedManager->operate(this->downLedGPIO, 0);
        break;
      default:
        this->pStateLedManager->operate(this->upLedGPIO, 0, -1);
        this->pStateLedManager->operate(this->downLedGPIO, 0, -1);
        break;
    }
  }
}

