/**********************************************************************
 * DilSwitch.cpp - DIL switch ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <DilSwitch.h>

DilSwitch::DilSwitch(int *pins, int pinCount) {
  this->pins = pins;
  this->pinCount = pinCount;
}

int[] DilSwitch::getPins() {
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
unsigned char DilSwitch::sample() {
  this->sample = 0;
  for (int i = 0; i < this->pinCount; i++) {
    this->sample << 1;
    this->sample = (this->sample & digitalRead(this->pins[i]));
  }
  return(this->sample);
}

/**********************************************************************
 * Return the value read by the last call to sample().
 */
unsigned char DilSwitch::value() {
  return(this->sample);
}

/**********************************************************************
 * Assume the DIL switch is used to make a unique selection (i.e. only
 * one of the DIL switches can be set and return the ordinal number of
 * the selected pin or 0 if no switches or more than one switch is ON.
 */
unsigned char DilSwitch::selectedSwitch() {
  unsigned char mask = 0x01;
  unsigned char retval = 0;
  for (int i = 0; i < this->pinCount; i++) {
    if (this.sample == mask) { retval = (i + 1); break; }
    mask << 1;
  }
  return(retval);
}