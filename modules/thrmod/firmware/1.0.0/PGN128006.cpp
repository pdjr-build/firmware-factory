/**********************************************************************
 * PGN128006.cpp (c) 2021 Paul Reeve <preeve@pdjr.eu>
 */

#include <NMEA2000StdTypes.h>
#include <N2kTypes.h>
#include "PGN128006.h"

PGN128006::PGN128006() {
  this->properties.ThrusterIdentifier = 0x00;
  this->properties.ThrusterDirectionControl = N2kDD473_OFF;
  this->properties.PowerEnable = N2kDD002_Off;
  this->properties.ThrusterRetractControl = N2kDD474_OFF;
  this->properties.SpeedControl = 0;
  this->properties.ThrusterControlEvents.SetEvents(0);
  this->properties.CommandTimeout = 0.25;
  this->properties.AzimuthControl = 0.0;
}

PGN128006_Properties PGN128006::getProperties() {
  return this->properties;
}

uint8_t PGN128006::getThrusterIdentifier() {
  return this->properties.ThrusterIdentifier;
}

tN2kDD473 PGN128006::getThrusterDirectionControl() {
  return this->properties.ThrusterDirectionControl;
}

tN2kDD002 PGN128006::getPowerEnable() {
  return this->properties.PowerEnable;
}

tN2kDD474 PGN128006::getThrusterRetractControl() {
  return this->properties.ThrusterRetractControl;
}

uint8_t PGN128006::getSpeedControl() {
  return this->properties.SpeedControl;
}

tN2kDD475 PGN128006::getThrusterControlEvents() {
  return this->properties.ThrusterControlEvents;
}

double PGN128006::getCommandTimeout() {
  return this->properties.CommandTimeout;
}

double PGN128006::getAzimuthControl() {
  return this->properties.AzimuthControl;
}

PGN128006_GenericField PGN128006::getField(int index) {
  PGN128006_GenericField retval = { 0 };
  switch (index) {
    case PGN128006_ThrusterIdentifier_FieldIndex: retval.F02 = this->properties.ThrusterIdentifier; break;
    case PGN128006_ThrusterDirectionControl_FieldIndex: retval.F03 = this->properties.ThrusterDirectionControl; break;
    case PGN128006_PowerEnable_FieldIndex: retval.F04 = this->properties.PowerEnable; break;
    case PGN128006_ThrusterRetractControl_FieldIndex: retval.F05 = this->properties.ThrusterRetractControl; break;
    case PGN128006_SpeedControl_FieldIndex: retval.F06 = this->properties.SpeedControl; break;
    case PGN128006_ThrusterControlEvents_FieldIndex: retval.F07 = this->properties.ThrusterControlEvents; break;
    case PGN128006_CommandTimeout_FieldIndex: retval.F08 = this->properties.CommandTimeout; break;
    case PGN128006_AzimuthControl_FieldIndex: retval.F09 = this->properties.AzimuthControl; break;
    default: break;
  }
  return retval;
}

void PGN128006::setProperties(PGN128006_Properties value) {
  this->properties = value;
}

void PGN128006::setThrusterIdentifier(uint8_t value) {
  this->properties.ThrusterIdentifier = value;
}

void PGN128006::setThrusterDirectionControl(tN2kDD473 value) {
  this->properties.ThrusterDirectionControl = value;
}

void PGN128006::setPowerEnable(tN2kDD002 value) {
  this->properties.PowerEnable = value;
}

void PGN128006::setThrusterRetractControl(tN2kDD474 value) {
  this->properties.ThrusterRetractControl = value;
}

void PGN128006::setSpeedControl(uint8_t value) {
  this->properties.SpeedControl = value;
}

void PGN128006::setThrusterControlEvents(tN2kDD475 value) {
  this->properties.ThrusterControlEvents = value;
}

void PGN128006::setCommandTimeout(double value) {
  this->properties.CommandTimeout = value;
}

void PGN128006::setAzimuthControl(double value) {
  this->properties.AzimuthControl = value;
}

void PGN128006::setField(int index, PGN128006_GenericField value) {
  switch (index) {
    case PGN128006_ThrusterIdentifier_FieldIndex: this->properties.ThrusterIdentifier = value.F02; break;
    case PGN128006_ThrusterDirectionControl_FieldIndex: this->properties.ThrusterDirectionControl = value.F03; break;
    case PGN128006_PowerEnable_FieldIndex: this->properties.PowerEnable = value.F04; break;
    case PGN128006_ThrusterRetractControl_FieldIndex: this->properties.ThrusterRetractControl = value.F05; break;
    case PGN128006_SpeedControl_FieldIndex: this->properties.SpeedControl = value.F06; break;
    case PGN128006_ThrusterControlEvents_FieldIndex: this->properties.ThrusterControlEvents = value.F07; break;
    case PGN128006_CommandTimeout_FieldIndex: this->properties.CommandTimeout = value.F08; break;
    case PGN128006_AzimuthControl_FieldIndex: this->properties.AzimuthControl = value.F09; break;
    default: break;
  }
}
