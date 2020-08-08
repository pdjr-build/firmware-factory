//*********************************************************************
// ElectricWindlass.cpp - class representing a windlass.
//
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#include "ElectricWindlass.h"

ElectricWindlass::ElectricWindlass(ElectricWindlassSettings settings) :
  Windlass(settings.windlassSettings) {
  this->settings = settings;
  this->state = ElectricWindlassStates_UNKNOWN;
  this->controllerVoltage = this->settings.nominalControllerVoltage;
  this->motorCurrent = this->settings.nominalMotorCurrent;
}

ElectricWindlassSettings ElectricWindlass::getElectricWindlassSettings() {
  return(this->settings);
}

void ElectricWindlass::setElectricWindlassState(ElectricWindlassStates state) {
  this->state = state;
}

ElectricWindlassStates ElectricWindlass::getElectricWindlassState() {
  return(this->state);
}

void ElectricWindlass::setControllerVoltage(double voltage) {
  this->controllerVoltage = voltage;
}

double ElectricWindlass::getControllerVoltage() {
  return(this->controllerVoltage);
}

bool ElectricWindlass::isControllerUnderVoltage() {
  return(this->controllerVoltage < this->settings.nominalControllerVoltage);
}

void ElectricWindlass::setMotorCurrent(double current) {
  this->motorCurrent = current;
}

double ElectricWindlass::getMotorCurrent() {
  return(this->motorCurrent);
}

bool ElectricWindlass::isMotorOverCurrent() {
  return(this->motorCurrent > this->settings.nominalMotorCurrent);
}
