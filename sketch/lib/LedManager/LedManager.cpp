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
 
LedManager::LedManager(unsigned long heartbeat, unsigned int interval) {
  this->heartbeat = heartbeat;
  this->interval = interval;
  this->timeout = 0UL;
  this->leds = NULL;
}

/**********************************************************************
 * Operate the LED on pin <gpio> and optionally automate its behaviour.
 *
 * @param gpio - the GPIO pin to which the LED is connected.
 * @param state - the state in which to leave the LED.
 * @param flashes - the required number of flashes (-1 says infinite)
 */
  
void LedManager::operate(unsigned int gpio, unsigned int state, int flashes) {
  Led *led = NULL;
  for (Led *l = this->leds; l != NULL; l = l->next) { if (l->gpio == gpio) led = l; }
  if (led == NULL) { led = new Led; led->next = this->leds; this->leds = led; }
  led->gpio = gpio;
  led->state = state;
  led->flashes = (flashes * 2);
  led->current = abs(led->flashes);
  if (led->flashes == 0) digitalWrite(gpio, led->state);
}

/**********************************************************************
 * Operate the defined leds.  Simply call from loop().
 */

void LedManager::loop() {
  unsigned long now = millis();
  if (now > this->timeout) {
    for (Led *led = this->leds; led != NULL; led = led->next) {
      if (led->flashes != 0) {
        if (led->current < 0) {
          digitalWrite(led->gpio, led->state);
          if ((led->flashes < 0) && (led->current <= (-1 * (int) this->interval))) led->current = abs(led->flashes);
        } else {
          if (led->current >= 0) {
            digitalWrite(led->gpio, (led->current % 2)?HIGH:LOW);
          }
        }
        led->current--;
        //Serial.println(led->current);
      }
    }
    this->timeout = (now + this->heartbeat);
  }
}
