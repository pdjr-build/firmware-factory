/**
 * N2kSpudpole.cpp 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling NMEA 2000 enabled spudpoles from the manufacturer Ankreo.
 */
 
#include <cstddef>
#include <string.h>
#include "N2kSpudpole.h"

N2kSpudpole::N2kSpudpole(N2kSpudpoleSettings settings):
  Spudpole(settings.spudpoleSettings) {  
    this->settings = settings;
    this->currentCommand = tN2kWindlassDirectionControl_OFF;
    this->commandTimeout = this->settings.defaultCommandTimeout;
    this->sequenceId = 0;
    this->executeCommand();
}

N2kSpudpoleSettings N2kSpudpole::getSettings() {
  return(this->settings);
}

void N2kSpudpole::setCommandTimeout(double seconds) {
  this->controlTimeout = seconds;
}

double N2kSpudpole::getCommandTimeout() {
  return(this->controlTimeout);
}

void incrSequenceId() {
  this->sequenceId++;
}

void populatePGN128776(tN2kMsg &N2kMsg) {
  tN2kDD478 events { false };
  SetN2kPGN128776(
    N2kMsg,
    this->sequenceId,
    this->settings.instance,
    this->currentCommand,
    N2kDD002_On,                // Anchor docking control is always enabled
    N2kDD488_SingleSpeed,       // These spudpoles are always single speed
    100,                        // Always single speed maximum
    N2kDD002_Unavailable,       // Power is always enabled
    N2kDD002_Unavailable,       // Mechanical locking is unavailable
    N2kDD002_Unavailable,       // Deck and anchor wash is unavailable
    N2kDD002_Unavailable,       // Anchor light is unavailable
    this->commandTimeout,
    events
  );
  return(N2kMsg);
}

void populatePGN128777(tN2kMsg &N2kMsg) {
  tN2kDD483 events {
    false,
    false,
    (this->getCounter() == 0),
    (this->getCounter() < this->settings.settings.settings.settings.turnsPerLayer),
    (this->getDeployedLineLength() >= this->settings.settings.settings.settings.usableLineLength)
  };
  SetN2kPGN128777(
    N2kMsg,
    this->sequenceId,
    this->settings.instance,
    (tN2kWindlassMotionStates) this-getState(),
    tN2kDD481_RopePresentlyDetected,
    this->getDeployedLineLength(),
    this->getLineSpeed(),
    (this->dockedStatus)?DD482_FullyDocked:DD482_NotDocked,
    events
  );
  return(N2kMsg);
}

void populatePGN128778(tN2kMsg &N2kMsg) {
  tN2kDD477 events;
  SetN2kPGN128778(
    N2kMsg,
    this->sequenceId,
    this->settings.instance,
    
   

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
