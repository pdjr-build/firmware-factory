//*********************************************************************
// N2kSpudpole.h 20 July 2020 (c) Paul Reeve <preeve@pdjr.eu>
//
// Abstract data type modelling an NMEA 2000 connected spudpoles.
//
// This ADT builds on the Spudpole class, adding methods for reporting
// the state of a spudpole in an N2K compliant fashion and also for
// controlling external spudpole hardware over N2K.
//
//*********************************************************************

#ifndef N2KSPUDPOLE_H
#define N2KSPUDPOLE_H

#include "N2kTypes.h"
#include "Spudpole.h"

struct N2kSpudpoleSettings {
  SpudpoleSettings spudpoleSettings;
  unsigned char instance;
  void (*controlCallback)(int);
  void (*controlTimerCallback)(unsigned long);
  unsigned long defaultControlTimeout;
};
typedef struct N2kSpudpoleSettings N2kSpudpoleSettings; 

class N2kSpudpole: public Spudpole {
  public:
    N2kSpudpole(N2kSpudpoleSettings settings);
    N2kSpudpoleSettings getSettings();
    void setControlTimeout(double seconds);
    double getControlTimeout();
    void incrSequenceId();
    tN2kMsg *getPGN128776(tN2kMsg &N2kMsg);
    tN2kMsg *getPGN128777(tN2kMsg &N2kMsg);
    tN2kMsg *getPGN128778(tN2kMsg &N2kMsg);
   private:
    N2kSpudpoleSettings settings;
    unsigned char sequenceId;
    tN2kWindlassDirectionControl currentCommand;
    double controlTimeout;
    void deploy();
    void retrieve();
    void stop();    
};

#endif
