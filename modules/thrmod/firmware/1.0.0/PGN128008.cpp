/**********************************************************************
 * PGN128008.cpp (c) 2021 Paul Reeve <preeve@pdjr.eu>
 */

#include <NMEA2000StdTypes.h>
#include <N2kTypes.h>
#include "PGN128008.h"

PGN128008::PGN128008() {
  this->properties.ThrusterIdentifier = 0x00;
  this->properties.ThrusterMotorEvents.SetEvents(0);
  this->properties.MotorCurrent = 0;
  this->properties.MotorTemperature = 0.0;
  this->properties.TotalMotorOperatingTime = 0.0;
}

PGN128008_Properties PGN128008::getProperties() {
  return this->properties;
}

unsigned char PGN128008::getThrusterIdentifier() {
  return this->properties.ThrusterIdentifier;
}

tN2kDD471 PGN128008::getThrusterMotorEvents() {
  return this->properties.ThrusterMotorEvents;
}

unsigned char PGN128008::getMotorCurrent() {
  return this->properties.MotorCurrent;
}

double PGN128008::getMotorTemperature() {
  return this->properties.MotorTemperature;
}

unsigned long PGN128008::getTotalMotorOperatingTime() {
  return this->properties.TotalMotorOperatingTime;
}

PGN128008_GenericField PGN128008::getField(int index) {
  PGN128008_GenericField retval = { 0 };
  switch (index) {
    case PGN128008_ThrusterIdentifier_FieldIndex: retval.F02 = this->properties.ThrusterIdentifier; break;
    case PGN128008_ThrusterMotorEvents_FieldIndex: retval.F03 = this->properties.ThrusterMotorEvents; break;
    case PGN128008_MotorCurrent_FieldIndex: retval.F04 = this->properties.MotorCurrent; break;
    case PGN128008_MotorTemperature_FieldIndex: retval.F05 = this->properties.MotorTemperature; break;
    case PGN128008_TotalMotorOperatingTime_FieldIndex: retval.F06 = this->properties.TotalMotorOperatingTime; break;
    default: break;
  }
  return retval;
}

void PGN128008::setProperties(PGN128008_Properties value) {
  this->properties = value;
}

void PGN128008::setThrusterIdentifier(unsigned char value) {
  this->properties.ThrusterIdentifier = value;
}

void PGN128008::setThrusterMotorEvents(tN2kDD471 value) {
  this->properties.ThrusterMotorEvents = value;
}

void PGN128008::setMotorCurrent(unsigned char value) {
  this->properties.MotorCurrent = value;
}

void PGN128008::setMotorTemperature(double value) {
  this->properties.MotorTemperature = value;
}

void PGN128008::setTotalMotorOperatingTime(unsigned long value) {
  this->properties.TotalMotorOperatingTime = value;
}

void PGN128008::setField(int index, PGN128008_GenericField value) {
  switch (index) {
    case PGN128008_ThrusterIdentifier_FieldIndex: this->properties.ThrusterIdentifier = value.F02; break;
    case PGN128008_ThrusterMotorEvents_FieldIndex: this->properties.ThrusterMotorEvents = value.F03; break;
    case PGN128008_MotorCurrent_FieldIndex: this->properties.MotorCurrent = value.F04; break;
    case PGN128008_MotorTemperature_FieldIndex: this->properties.MotorTemperature = value.F05; break;
    case PGN128008_TotalMotorOperatingTime_FieldIndex: this->properties.TotalMotorOperatingTime = value.F06; break;
    default: break;
  }
}

void PGN128008::bumpTotalMotorOperatingTime(unsigned long seconds) {
  this->properties.TotalMotorOperatingTime += seconds;
}
