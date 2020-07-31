/**
 * N2kSpudpole.h 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling NMEA 2000 enabled spudpoles from the manufacturer Ankreo.
 */

#ifndef N2KSPUDPOLE_H
#define N2KSPUDPOLE_H

#include "../NMEA2000/src/N2kTypes.h"

class N2kSpudpole: public Spudpole {
  public:
    N2kSpudpole(unsigned char instance);
    void configureCommandTimeout(void (*controlTimerCallback)(unsigned long), unsigned long controlTimeout);
    unsigned char getInstance();
    void deploy();
    void retrieve();
    tN2kDD477 getWindlassMonitoringEvents();
    tN2kDD480 getWindlassMotionStatus();
    tN2kDD481 getRodeTypeStatus();
    tN2kDD482 getAnchorDockingStatus();
    tN2kDD483 getWindlassOperatingEvents();
    tN2kDD484 getWindlassDirectionControl();
    double getRodeCounterValue();
    double getWindlassLineSpeed();
  private:
    unsigned char instance;
    void (*controlTimerCallback)(unsigned long);
    unsigned long controlTimeout;
};

#endif
