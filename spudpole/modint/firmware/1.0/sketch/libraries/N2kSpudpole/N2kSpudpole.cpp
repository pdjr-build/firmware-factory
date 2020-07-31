/**
 * N2kSpudpole.cpp 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling NMEA 2000 enabled spudpoles from the manufacturer Ankreo.
 */
 
#include <cstddef>
#include <string.h>
#include "submodules/Spudpole/Spudpole.h"
#include "N2kSpudpole.h"

N2kSpudpole::N2kSpudpole(unsigned char instance):
  Spudpole() {
  this->instance = instance;
  this->controlTimerCallback = 0;
  this->controlTimeout = 0L;
}

void N2kSpudpole::configureCommandTimeout(void (*controlTimerCallback)(unsigned long), unsigned long controlTimeout) {
  this->controlTimerCallback = controlTimerCallback;
  this->controlTimeout = controlTimeout;
}


unsigned char N2kSpudpole::getInstance() {
  return(this->instance);
}

void N2kSpudpole::deploy() {
  Spudpole::deploy();
  if (this->controlTimerCallback) this->controlTimerCallback(this->controlTimeout);
}

void N2kSpudpole::retrieve() {
  Spudpole::retrieve();
  if (this->controlTimerCallback) this->controlTimerCallback(this->controlTimeout);
}

tN2kDD477 N2kSpudpole::getWindlassMonitoringEvents() {
  tN2kDD477 retval = N2kDD477_NoErrorsPresent;
  return(retval);
}

tN2kDD480 N2kSpudpole::getWindlassMotionStatus() {
  tN2kDD480 retval = N2kDD480_Unavailable;
  switch (this->getState()) {
    case SpudpoleState_UNKNOWN: retval = N2kDD480_Unavailable; break;
    case SpudpoleState_DOCKED: retval = N2kDD480_WindlassStopped; break;
    case SpudpoleState_DEPLOYING: retval = N2kDD480_DeploymentOccurring; break;
    case SpudpoleState_RETRIEVING: retval = N2kDD480_RetrievalOccurring; break;
    case SpudpoleState_STOPPED: retval = N2kDD480_WindlassStopped; break;
  }
  return(retval);
}

tN2kDD481 N2kSpudpole::getRodeTypeStatus() {
  tN2kDD481 retval = N2kDD481_RopePresentlyDetected;
  return(retval);
}

tN2kDD482 N2kSpudpole::getAnchorDockingStatus() {
  tN2kDD482 retval = N2kDD482_DataNotAvailable;
  switch (this->getState()) {
    case SpudpoleState_UNKNOWN: retval = N2kDD482_DataNotAvailable; break;
    case SpudpoleState_DOCKED: retval = N2kDD482_FullyDocked; break;
    case SpudpoleState_DEPLOYING: retval = N2kDD482_NotDocked; break;
    case SpudpoleState_RETRIEVING: retval = N2kDD482_NotDocked; break;
    case SpudpoleState_STOPPED: retval = N2kDD482_NotDocked; break;
  }
  return(retval);
}

tN2kDD483 N2kSpudpole::getWindlassOperatingEvents() {
  tN2kDD483 retval = N2kDD483_NoErrorsOrEventsPresent;
  return(retval);
}

tN2kDD484 N2kSpudpole::getWindlassDirectionControl() {
  tN2kDD484 retval = N2kDD484_Off;
  switch (this->getState()) {
    case SpudpoleState_UNKNOWN: retval = N2kDD484_Off; break;
    case SpudpoleState_DOCKED: retval = N2kDD484_Off; break;
    case SpudpoleState_DEPLOYING: retval = N2kDD484_Down; break;
    case SpudpoleState_RETRIEVING: retval = N2kDD484_Up; break;
    case SpudpoleState_STOPPED: retval = N2kDD484_Off; break;
  }
  return(retval);
}

double N2kSpudpole::getRodeCounterValue() {
  return(Spudpole::getDeployedLineLength());
}

double N2kSpudpole::getWindlassLineSpeed() {
  return(0.6);
}
