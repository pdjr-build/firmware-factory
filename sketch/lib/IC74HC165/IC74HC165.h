/**********************************************************************
 * IC74HC165.h - ADT for accessing a 74HC165 PISO buffer.
 * 2022 (c) Paul Reeve <preeve@pdjr.eu>
 * 
 * * 
 * The 74HC165 is an 8-bit parallel-to-serial I/O buffer. This library
 * allows the host application to read the buffer.
 * 
 * Example:
 * 
 * #define PisoDataGpio 2
 * #define PisoLatchGpio 3
 * #define PisoClockGpio 4
 * #define MyInterestingBit 6
 * 
 * IC74HC165 piso = IC74HC165(PisoDataGpio, PisoLatchGpio, PisoClockGpio);
 * 
 * void setup() {
 *   piso.begin();
 * }
 * 
 * void loop() {
 *   uint8_t byte;
 *   int bit;
 *
 *   byte = piso.readByte();
 *   bit = piso.readBit(MyInterestingBit); 
 * }
 * 
 */

#ifndef IC74HC165_H
#define IC74HC165_H

class IC74HC165 {
  public:
    IC74HC165(uint8_t gpioData, uint8_t gpioLatch, uint8_t gpioClock);
    void begin();
    uint8_t readByte();
    int readBit(int bit);
  private:
    uint8_t gpioData;
    uint8_t gpioLatch;
    uint8_t gpioClock
};

#endif