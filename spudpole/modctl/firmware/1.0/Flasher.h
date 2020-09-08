//*********************************************************************
// Flasher.h - to manage flashing leds.
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#ifndef FLASHER_H
#define FLASHER_H

class Flasher {
  public:
    Flasher(unsigned long heartbeat = 200, unsigned long refactory = 1000);
    void flash(unsigned int gpio, unsigned int flashes = 0, unsigned int endstate = 0);
    void operate();
  private:
    struct Led {
      unsigned int gpio;
      unsigned int flashes;
      unsigned int endstate;
      int current;
      Led *next;
    };
    unsigned long heartbeat;
    unsigned long refactory;
    Led *leds;
};

#endif
