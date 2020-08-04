//*********************************************************************
// ElectricWindlass.cpp - class representing a windlass.
//
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#include "ElectricWindlass.h"

ElectricWindlass::ElectricWindlass(ElectricWindlassSettings settings) :
  Windlass(settings.windlassSettings) {
  this->settings = settings;
  this->controllerVoltage = this->settings.nominalControllerVoltage;
  this->motorCurrent = this->settings.nominalMotorCurrent;
}

ElectricWindlassSettings ElectricWindlass::getSettings() {
  return(this->settings);
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
