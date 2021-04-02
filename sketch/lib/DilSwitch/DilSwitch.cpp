/**********************************************************************
 * DilSwitch.cpp - DIL switch ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <Arduino.h>
#include <DilSwitch.h>

DilSwitch::DilSwitch(int *pins, int pinCount) {
  this->pins = pins;
  this->pinCount = pinCount;
  this->lastsample = 0;
}

int *DilSwitch::getPins() {
  return(this->pins);
}

int DilSwitch::getPinCount() {
  return(this->pinCount);
}

/**********************************************************************
 * Reads the state of the GPIO pins and returns the represented value
 * as an integer. The returned value is saved and can be subsequently
 * recovered using the value(), selection() and pinState() functions
 * without triggering further GPIO reads.
 */
DilSwitch *DilSwitch::sample() {
  this->lastsample = 0;
  for (int i = (this->pinCount - 1); i >= 0; i--) {
    this->lastsample = (this->lastsample | (((digitalRead(this->pins[i]) == 0)?1:0) << i));
  }
  return(this);
}

/**********************************************************************
 * Return the value read by the last call to sample().
 */
unsigned char DilSwitch::value() {
  return(this->lastsample);
}

/**********************************************************************
 * Returns the ordinal number of a singly selected DIL switch or 0 if
 * no switches or more than one switch is selected.
 */
unsigned char DilSwitch::selectedSwitch() {
  switch (this->lastsample) {
    case 1: return(1); break;
    case 2: return(2); break;
    case 4: return(3); break;
    case 8: return(4); break;
    case 16: return(5); break;
    case 32: return(6); break;
    case 64: return(7); break;
    case 128: return(8); break;
    default: return(0); break;
  }
}
