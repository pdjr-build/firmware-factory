/**
 * Spudpole.cpp 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling spudpoles from the manufacturer Ankreo.
 */

#include <cstddef>
#include <string.h>
#include "Spudpole.h"

Spudpole::Spudpole() {
  this->controllerVoltage = 0.0;
  this->motorCurrent = 0.0;
  this->state = SpudpoleState_UNKNOWN;
  this->counter = 0;
  this->controlCallback = 0;
  this->spoolDiameter = 0.0;
  this->lineDiameter = 0.0;
  this->spoolWidth = 0;
  this->lineTurnsWhenDocked = 0;
  this->motorRunTime = 0L;
  this->timerCallback = 0;
}

void Spudpole::setControlCallback(void (*controlCallback)(SpudpoleControl)) {
  this->controlCallback = controlCallback;
}

void Spudpole::configureLineMeasurement(double spoolDiameter, double lineDiameter, unsigned int spoolWidth, unsigned int lineTurnsWhenDocked) {
  this->spoolDiameter = spoolDiameter;
  this->lineDiameter = lineDiameter;
  this->spoolWidth = spoolWidth;
  this->lineTurnsWhenDocked = lineTurnsWhenDocked;
}

void Spudpole::configureRunTimeAccounting(unsigned long motorRunTime, unsigned long (*timerCallback)(SpudpoleTimer, unsigned long)) {
  this->motorRunTime = motorRunTime;
  this->timerCallback = timerCallback;
}

void Spudpole::setControllerVoltage(double controllerVoltage) {
  this->controllerVoltage = controllerVoltage;
}

void Spudpole::setMotorCurrent(double motorCurrent) {
  this->motorCurrent = motorCurrent;
}

double Spudpole::getControllerVoltage() {
  return(this->controllerVoltage);
}

double Spudpole::getMotorCurrent() {
  return(this->motorCurrent);
}

void Spudpole::setDocked() {
  this->state = SpudpoleState_DOCKED;
  this->counter = 0;
}

void Spudpole::setStopped() {
  this->state = SpudpoleState_STOPPED;
  if (this->controlCallback) this->controlCallback(SpudpoleControl_STOP);
  if (this->timerCallback) this->motorRunTime = this->timerCallback(SpudpoleTimer_STOP, this->motorRunTime);
}

void Spudpole::deploy() {
  this->state = SpudpoleState_DEPLOYING;
  if (this->controlCallback) this->controlCallback(SpudpoleControl_DEPLOY);
  if (this->timerCallback) this->timerCallback(SpudpoleTimer_START, this->motorRunTime);
}

void Spudpole::retrieve() {
  this->state = SpudpoleState_RETRIEVING;
  if (this->controlCallback) this->controlCallback(SpudpoleControl_RETRIEVE);
  if (this->timerCallback) this->timerCallback(SpudpoleTimer_START, this->motorRunTime);
}

void Spudpole::stop() {
  this->setStopped();
}

SpudpoleState Spudpole::getState() {
  return(this->state);
}

bool Spudpole::isWorking() {
  return((this->state == SpudpoleState_DEPLOYING) || (this->state == SpudpoleState_RETRIEVING));
}

bool Spudpole::isDocked() {
  return(this->state == SpudpoleState_DOCKED);
}

bool Spudpole::isDeployed() {
  return(this->state == SpudpoleState_STOPPED);
}

unsigned int Spudpole::getCounter() {
  return(this->counter);
}

unsigned int Spudpole::incrCounter() {
  if (this->state == SpudpoleState_DEPLOYING) this->counter++;
  return(this->counter);
}

unsigned int Spudpole::decrCounter() {
  if ((this->state == SpudpoleState_RETRIEVING) && (this->counter > 0)) this->counter--;
  return(this->counter);
}

unsigned int Spudpole::bumpCounter() {
  switch (this->state) {
    case SpudpoleState_UNKNOWN: break;
    case SpudpoleState_DOCKED: break;
    case SpudpoleState_DEPLOYING: this->incrCounter(); break;
    case SpudpoleState_RETRIEVING: this->decrCounter(); break;
    case SpudpoleState_STOPPED: break;
  }
  return(this->counter);
}

double Spudpole::getDeployedLineLength() {
  double retval = -1.0;
  if ((this->state != SpudpoleState_UNKNOWN) && (this->spoolDiameter > 0.0)) {
    retval = 0.0;
    if (this->counter >= 0) {
        retval = this->lineLengthFromCounter(this->lineTurnsWhenDocked) - this->lineLengthFromCounter(this->lineTurnsWhenDocked - this->counter);
    }
  }
  return(retval);
}

unsigned long Spudpole::getMotorRunTime() {
  return((this->timerCallback)?this->motorRunTime:0L);
}

//*****************************************************************************
// Private methods
//*****************************************************************************

double Spudpole::lineLengthFromCounter(int counter) {
  double retval = 0.0;
  double rlol;
  int layersUsed = (counter / this->spoolWidth);
  for (int layer = 0; layer <= layersUsed; layer++) {
    int turnsOnLayer = (layer < layersUsed)?this->spoolWidth:(counter % this->spoolWidth); 
    rlol = this->lineLengthOnLayer(layer, turnsOnLayer);
    retval += rlol;
  }
  return(retval);
}

double Spudpole::lineLengthOnLayer(int layer, int turnsOnLayer) {
  double retval = turnsOnLayer * (3.1416 * (this->spoolDiameter + this->lineDiameter + (layer * 2 * this->lineDiameter)));
  return(retval);
}

