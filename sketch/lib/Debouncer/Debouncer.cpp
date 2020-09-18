/**********************************************************************
 * Debouncer.cpp - 8-channel GPIO switch debouncer.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <Debouncer.h>
#include <Arduino.h>

Debouncer::Debouncer(int* gpios, unsigned long interval) {
  for (unsigned int i = 0; i < ARRAYSIZE(this->gpios); i++) this->gpios[i] = (i < ARRAYSIZE(gpios))?gpios[i]:-1;
  this->interval = interval;
  this->deadline = 0L;
}

void Debouncer::debounce() {
  unsigned long now = millis();
  if (now > this->deadline) {
    this->switches.state.channel0 = (this->gpios[0] >= 0)?digitalRead(this->gpios[0]):0;
    this->switches.state.channel1 = (this->gpios[1] >= 0)?digitalRead(this->gpios[1]):0;
    this->switches.state.channel2 = (this->gpios[2] >= 0)?digitalRead(this->gpios[2]):0;
    this->switches.state.channel3 = (this->gpios[3] >= 0)?digitalRead(this->gpios[3]):0;
    this->switches.state.channel4 = (this->gpios[4] >= 0)?digitalRead(this->gpios[4]):0;
    this->switches.state.channel5 = (this->gpios[5] >= 0)?digitalRead(this->gpios[5]):0;
    this->switches.state.channel6 = (this->gpios[6] >= 0)?digitalRead(this->gpios[6]):0;
    this->switches.state.channel7 = (this->gpios[7] >= 0)?digitalRead(this->gpios[7]):0;
    this->switches.states = debounceStates(this->switches.states);
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
  for (unsigned int i = 0; i < ARRAYSIZE(this->gpios); i++) {
    if (this->gpios[i] == gpio) { index = i; break; }
  }
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
}

