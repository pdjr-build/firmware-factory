/**********************************************************************
 * LedManager.h - manage an arbitrary number of leds.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 * 
 * LedManager acts as an interface between an application and its use
 * of connected LEDs (well actually just the use of the GPIO pins to
 * which LEDs might be attached).
 * 
 * The manager allows the application to operate an LED by switching
 * it on or off, making it flash a certain number of times and then
 * adopt a particulare state.
 * 
 * To begin using LedManager, create a new instance:
 * 
 * LedManager myManager = new LedManager();
 * 
 * And to operate an LED specify the gpio pin and state required:
 * 
 * myManager.operate(13,1); // turn the LED on pin 13 on
 * 
 * If you specify a third argument, then the LED will flash that number
 * of times before adopting the specified state:
 * 
 * myManager.operate(13,0,3); // flash the LED three times, then turn it off
 * 
 * If you want the flashing cycle to repeat, then make the flash count
 * a negative number:
 * 
 * myManager.operate(13,0,-3);
 */

#ifndef LEDMANAGER_H
#define LEDMANAGER_H

class LedManager {
  public:
    LedManager(unsigned long heartbeat = 200, unsigned int interval = 5);
    void operate(unsigned int gpio, unsigned int state, int flashes = 0);
    void loop();
  private:
    struct Led {
      unsigned int gpio;
      unsigned int state;
      int flashes;
      int current;
      Led *next;
    };
    unsigned long heartbeat;
    unsigned int interval;
    unsigned long timeout;
    Led *leds;
};

#endif
