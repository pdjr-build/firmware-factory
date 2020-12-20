/**********************************************************************
 * Debouncer.cpp - 8-channel GPIO switch debouncer.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <Debouncer.h>
#include <Arduino.h>

Debouncer::Debouncer(int gpios[], unsigned long interval) {
  for (unsigned int i = 0; i < DEBOUNCER_SIZE; i++) this->gpios[i] = gpios[i];
  this->interval = interval;
  this->deadline = 0UL;
}

void Debouncer::debounce() {
  unsigned long now = millis();
  unsigned char sample = 0;
  if (now > this->deadline) {
    for (unsigned int i = 0; i < DEBOUNCER_SIZE; i++) {
      sample = (sample | (((this->gpios[i] >= 0)?digitalRead(this->gpios[i]):0) << i));
    }
    this->switches.states = debounceStates(sample);
    this->deadline = (now + this->interval);
  }
}

unsigned char Debouncer::debounceStates(unsigned char sample) {
  static unsigned char state, cnt0, cnt1;
  unsigned char delta;

  delta = sample ^ state;
  cnt1 = (cnt1 ^ cnt0) & (delta & sample);
  cnt0 = ~cnt0 & (delta & sample);
  state ^= (delta & ~(cnt0 | cnt1));
  return state;
}

bool Debouncer::channelState(int gpio) {
  int index = -1;
  for (unsigned int i = 0; i < DEBOUNCER_SIZE; i++) {
    if (this->gpios[i] == gpio) { index = i; break; }
  }
  return((index >= 0)?((this->switches.states >> index) & 0x01):0);
  /*
  switch (index) {
    case 0: return((bool) this->switches.state.channel0); break;
    case 1: return((bool) this->switches.state.channel1); break;
    case 2: return((bool) this->switches.state.channel2); break;
    case 3: return((bool) this->switches.state.channel3); break;
    case 4: return((bool) this->switches.state.channel4); break;
    case 5: return((bool) this->switches.state.channel5); break;
    case 6: return((bool) this->switches.state.channel6); break;
    case 7: return((bool) this->switches.state.channel7); break;
    default: return(false); break;
  }
  */
}

void Debouncer::dumpConfiguration() {
  for (unsigned int i = 0; i < DEBOUNCER_SIZE; i++) {
    Serial.print("Debouncer channel "); Serial.print(i); Serial.print(": "); Serial.println(this->gpios[i]);
  }
  Serial.print("State: "); Serial.println(this->switches.states, BIN);
}

unsigned char Debouncer::getStates() {
  return(this->switches.states);
}
