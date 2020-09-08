/**********************************************************************
 * Flasher.cpp - to manage flashing leds.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include "flasher.h"

/**********************************************************************
 * Create a new Flasher instance with optional <heartbeat> and
 * <refactory> period, both in milliseconds.
 * <heartbeat> defines the flasher's basic period and applies to both
 * on and off phases whilst <refactory> defines the quiescent period
 * between flash cycles.
 */
 
Flasher::Flasher(unsigned long heartbeat, unsigned long refactory) {
  this.heartbeat = heartbeat;
  this.refactory = refactory;
  this.leds = NULL;
}

/**********************************************************************
 * Trigger Flasher behaviour on the LED connected to <gpio>.
 *
 * <flashes> specifies the number of flashes required. A value of zero
 * (the default) says "don't flash" and the LED state is determined by
 * <endstate>.
 *
 * <endstate> specifies the state in which the LED should be left when
 * any specified flashing sequence terminates.
 */
  
void Flasher::flash(unsigned int gpio, unsigned int flashes, unsigned int endstate) {
  Led *led = NULL;
  for (Led *l = this.leds; l != NULL; l = l->next) { if (l->gpio == gpio) led = l; }
  if (led == NULL) { led = new Led; led->next = this.leds; this.leds = led; }
  led->gpio = gpio;
  led->flashes = (flashes * 2);
  led->endstate = endstate;
  led->current = led->flashes;
  if (led->flashes == 0) digitalWrite(gpio, endstate);
}


void Flasher::operate() {
  static unsigned long timeout = 0UL;
  unsigned long now = millis();
  if (now > timeout) {
    for (Led *led = this.leds; led != NULL; led = led->next) {
      if (led->flashes != 0) {
        if (led->current == 0) {
          digitalWrite(led->gpio, led->endstate);
        } else {
          if (led->current >= 0) {
            digitalWrite(led->gpio, (led->current % 2)?HIGH:LOW);
          } else {
            if (led->current <= -(led->refactory)) {
              led->current = led->flashes;
            }
          }
        }
        led->current--;
      }
    }
    timeout = (now + this.heartbeat);
  }
}

