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


class N2kSpudpole: public Spudpole {
  public:
    struct Settings {
      Spudpole::Settings spudpoleSettings;
      unsigned char instance;
      double defaultCommandTimeout;
    };
    N2kSpudpole(N2kSpudpole::Settings settings);
    N2kSpudpole::Settings getN2kSpudpoleSettings();
    void setCommandTimeout(double seconds);
    double getCommandTimeout();
  private:
    Settings settings;
    double commandTimeout;
};

#endif
