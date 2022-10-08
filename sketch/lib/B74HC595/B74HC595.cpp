/**********************************************************************
 * B74HC595.cpp - operate a 74HC595 serial-parallel buffer.
 * 2022 (c) Paul Reeve.
 */

#include "B74HC595.h"

B74HC595::B74HC595(int gpioClock, int gpioData, int gpioLatch, unsigned char state, unsigned char defaultDirection) {
    this->gpioData = gpioClock;
    this->gpioData = gpioData;
    this->gpioLatch = gpioLatch;
    this->getStatus = 0;
    this->interval = 20;
    this->defaultDirection = defaultDirection;
    this->PREEMPT_FLAG = false;
    this->OVERRIDE_FLAG = false;

    this->update(state, this->defaultDirection);
}

void B74HC595::setDefaultDirection(unsigned char direction) {
    this->defaultDirection = (defaultDirection == 99)?LSBFIRST:defaultDirection;
}

void B74HC595::update(unsigned char state, unsigned char direction) {
    digitalWrite(this->gpioLatch, 0);
    shiftOut(this->gpioData, this->gpioClock, (direction == 99)?this->defaultDirection:direction, state);
    digitalWrite(this->gpioLatch, 1);
}

void B74HC595::enableLoopUpdates(unsigned char (*getStatus)(), unsigned long interval) {
    this->getStatus = getStatus;
    this->interval = interval;
}

void B74HC595::loop() {
    static unsigned long deadline = 0UL;
    unsigned long now = millis();

    if (((now > deadline) || this->PREEMPT_FLAG) && (!this->OVERRIDE_FLAG) && (this->interval) && (this->getStatus)) {
        this->update(this->getStatus());
        this->PREEMPT_FLAG = false;

        deadline = (now + this->interval);
    }
}

void B74HC595::preempt() {
    this->PREEMPT_FLAG = true;
}

void B74HC595::override(unsigned char state, unsigned char direction) {
    this->OVERRIDE_FLAG = true;
    this->update(state, (direction == 99)?this->defaultDirection:direction);
}

void B74HC595::cancelOverride() {
    this->OVERRIDE_FLAG = false;
}