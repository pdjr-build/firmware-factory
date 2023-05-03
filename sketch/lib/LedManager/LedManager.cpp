/**********************************************************************
 * LedManager.cpp - manage an arbitrary number of leds.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include "Arduino.h"
#include "LedManager.h"

/**********************************************************************
 * Create a new LedManager instance with optional <heartbeat> (in
 * milliseconds) and <interval> in heartbeats.
 * 
 * <heartbeat> defines the flasher's basic period and applies to both
 * on and off phases whilst <interval> defines the quiescent period
 * between flash cycles.
 */
LedManager::LedManager(void (*callback)(unsigned char status), unsigned long interval) {
  this->callback = callback;
  this->interval = interval;

  this>states = new int[LedManager::LED_COUNT];
  for (int i = 0; i < LedManager::LED_COUNT; i++) this->states[i] = LedManager::OFF;
  this->deadline = 0UL;
}

void LedManager::setLedState(unsigned int led, LedManager::Pattern pattern) {
  if (led < LedManager::LED_COUNT) this->states[led] = pattern;
}

void update() {
  unsigned long now = millis();
  unsigned int status = 0;

  if (now > this->deadline) {
    for (int i = 0; i < LedManager::LED_COUNT; i++) {
      status << 1;
      switch (this->status[i]) {
        case LedManager::THRICE: status &= 1; this->status[i] = LedManager::OFF_TWICE_NEXT; break;
        case LedManager::OFF_TWICE_NEXT: status &= 0; this->status[i] = LedManager::OFF_TWICE_NEXT; break;
        case LedManager::TWICE: status &= 1; this->status[i] = LedManager::OFF_ONCE_NEXT; break;
        case LedManager::OFF_ONCE_NEXT: status &= 0; this->status[i] = LedManager::OFF_ONCE_NEXT; break;
        case LedManager::ONCE: status &= 1; this->status[i] = LedManager::OFF; break;
        case LedManager::OFF: status &= 0; break;
      }
    }
    this->update(status);
    this->deadline = (now + this->interval);
  }
}
