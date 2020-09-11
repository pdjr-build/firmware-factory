/**********************************************************************
 * WindlassState.h - in memory cache of windlass run-time properties.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef WINDLASSSTATE_H
#define WINDLASSSTATE_H

#include <LedManager.h>

#define WINDLASSSTATE_DISABLED_INSTANCE_VALUE 0x7F

class WindlassState {
  public:
    enum State { DOCKED, DEPLOYING, DEPLOYED, RETRIEVING, UNKNOWN };
    WindlassState(unsigned char instance, int gpioStatusLed, int gpioUpLed, int gpioDownLed);
    void setLedManagers(LedManager *statusLedManager, LedManager *stateLedManager);
    void setInstance(unsigned char instance);
    void setAddress(unsigned char address);
    void setState(WindlassState::State state);
    unsigned char getInstance();
    unsigned char getAddress();
    bool isDisabled();
    bool isConfigured();
    bool isReady();
  private:
    unsigned char instance; 
    unsigned char address;
    LedManager *statusLedManager;
    LedManager *stateLedManager;
    int gpioStatusLed;
    int gpioUpLed;
    int gpioDownLed;
    State state;
    void updateStatusLed();
    void updateStateLed();
};

#endif
