/**********************************************************************
 * DilSwitch.h - DIL switch ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef DILSWITCH_H
#define DILSWITCH_H

class DilSwitch {
  public:
    DilSwitch(int *pins, int pinCount);
    int[] getPins();
    int getPinCount();

    unsigned char sample();
    unsigned char value();
    unsigned char selectedSwitch();
  private:
    int *pins;
    int pinCount;
    unsigned char sample;
};

#endif
