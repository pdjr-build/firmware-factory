/**********************************************************************
 * WindlassState.h - in memory cache of windlass run-time properties.
 * 2020 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef WINDLASSSTATE_H
#define WINDLASSSTATE_H

#include <Debouncer.h>
#include <LedManager.h>

#define WINDLASSSTATE_DISABLED_INSTANCE_VALUE 0x7F

class WindlassState {
  public:
    enum State { DOCKED, DEPLOYING, DEPLOYED, RETRIEVING, UNKNOWN };
    WindlassState();
    bool isDisabled();
    bool isConfigured();
    bool isReady();

    unsigned char instance; 
    unsigned char address;
    State state;
    
    int programmeSwitchGPIO;
    int upSwitchGPIO;
    int downSwitchGPIO;
    int statusLedGPIO;
    int upLedGPIO;
    int downLedGPIO;
    int instanceStorageAddress;

    Debouncer *pDebouncer;
    LedManager *pStatusLedManager;
    LedManager *pStateLedManager;
  private:
    void updateStatusLed();
    void updateStateLed();
};

#endif
