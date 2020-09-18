/**********************************************************************
 * Debouncer.h - 8 channel GPIO switch debouncer.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 * 
 * int channels[] = { 6, 10 }; // Debounce GPIO channels 6 & 10
 * Debouncer debouncer = new Debouncer(channels);
 * 
 * void loop() {
 *   debouncer->debounce();
 *   if (!debouncer->channelState(6)) {
 *     // do something when channel 6 goes low
 *   }
 * }
 */

#ifndef DEBOUNCER_H
#define DEBOUNCER_H

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

// Number of milliseconds between debounce samplings.
//
#define DEBOUNCER_INTERVAL 5UL

class Debouncer {
  public:
    Debouncer(int *gpios, unsigned long interval = DEBOUNCER_INTERVAL);
    void debounce();
    bool channelState(int gpio);
  private:
    union DEBOUNCED_SWITCHES_T {
      unsigned char states;
      struct {
        unsigned char channel0:1;
        unsigned char channel1:1;
        unsigned char channel2:1;
        unsigned char channel3:1;
        unsigned char channel4:1;
        unsigned char channel5:1;
        unsigned char channel6:1;
        unsigned char channel7:1;
      } state;
      DEBOUNCED_SWITCHES_T(): states(0XFF) {};
    } switches;
    int gpios[8];
    unsigned long interval;
    unsigned long deadline;
    unsigned char debounceStates(unsigned char sample);
};

#endif
