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

#include "N2kMsg.h"
#include "N2kTypes.h"
#include "Spudpole.h"

struct N2kSpudpoleSettings {
  SpudpoleSettings spudpoleSettings;
  unsigned char instance;
  double defaultCommandTimeout;
};

class N2kSpudpole: public Spudpole {
  public:
    N2kSpudpole(N2kSpudpoleSettings settings);
    N2kSpudpoleSettings getN2kSpudpoleSettings();
    void setCommandTimeout(double seconds);
    double getCommandTimeout();
    void incrSequenceId();
    void populatePGN128776(tN2kMsg &N2kMsg);
    void populatePGN128777(tN2kMsg &N2kMsg);
    void populatePGN128778(tN2kMsg &N2kMsg);
    void deploy();
    void retrieve();
    void stop();
  private:
    N2kSpudpoleSettings settings;
    unsigned char sequenceId;
    double commandTimeout;
};

#endif
