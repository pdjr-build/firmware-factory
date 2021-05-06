/**********************************************************************
 * PGN128007.cpp (c) 2021 Paul Reeve <preeve@pdjr.eu>
 */

#include <NMEA2000StdTypes.h>
#include <N2kTypes.h>
#include "PGN128007.h"

PGN128007::PGN128007() {
  this->properties.ThrusterIdentifier = 0x00;
  this->properties.ThrusterMotorType = N2kDD487_Hydraulic;
  this->properties.MotorPowerRating = 0;
  this->properties.MaximumMotorTemperatureRating = 0.0;
  this->properties.MaximumRotationalSpeed = 0.0;
}

PGN128007_Properties PGN128007::getProperties() {
  return this->properties;
}

unsigned char PGN128007::getThrusterIdentifier() {
  return this->properties.ThrusterIdentifier;
}

tN2kDD487 PGN128007::getThrusterMotorType() {
  return this->properties.ThrusterMotorType;
}

int PGN128007::getMotorPowerRating() {
  return this->properties.MotorPowerRating;
}

double PGN128007::getMaximumMotorTemperatureRating() {
  return this->properties.MaximumMotorTemperatureRating;
}

double PGN128007::getMaximumRotationalSpeed() {
  return this->properties.MaximumRotationalSpeed;
}

PGN128007_GenericField PGN128007::getField(int index) {
  PGN128007_GenericField retval = { 0 };
  switch (index) {
    case PGN128007_ThrusterIdentifier_FieldIndex: retval.F01 = this->properties.ThrusterIdentifier; break;
    case PGN128007_ThrusterMotorType_FieldIndex: retval.F02 = this->properties.ThrusterMotorType; break;
    case PGN128007_MotorPowerRating_FieldIndex: retval.F04 = this->properties.MotorPowerRating; break;
    case PGN128007_MaximumMotorTemperatureRating_FieldIndex: retval.F05 = this->properties.MaximumMotorTemperatureRating; break;
    case PGN128007_MaximumRotationalSpeed_FieldIndex: retval.F06 = this->properties.MaximumRotationalSpeed; break;
    default: break;
  }
  return retval;
}

void PGN128007::setProperties(PGN128007_Properties value) {
  this->properties = value;
}

void PGN128007::setThrusterIdentifier(unsigned char value) {
  this->properties.ThrusterIdentifier = value;
}

void PGN128007::setThrusterMotorType(tN2kDD487 value) {
  this->properties.ThrusterMotorType = value;
}

void PGN128007::setMotorPowerRating(int value) {
  this->properties.MotorPowerRating = value;
}

void PGN128007::setMaximumMotorTemperatureRating(double value) {
  this->properties.MaximumMotorTemperatureRating = value;
}

void PGN128007::setMaximumRotationalSpeed(double value) {
  this->properties.MaximumRotationalSpeed = value;
}

void PGN128007::setField(int index, PGN128007_GenericField value) {
  switch (index) {
    case PGN128007_ThrusterIdentifier_FieldIndex: this->properties.ThrusterIdentifier = value.F01; break;
    case PGN128007_ThrusterMotorType_FieldIndex: this->properties.ThrusterMotorType = value.F02; break;
    case PGN128007_MotorPowerRating_FieldIndex: this->properties.MotorPowerRating = value.F04; break;
    case PGN128007_MaximumMotorTemperatureRating_FieldIndex: this->properties.MaximumMotorTemperatureRating = value.F05; break;
    case PGN128007_MaximumRotationalSpeed_FieldIndex: this->properties.MaximumRotationalSpeed = value.F06; break;
    default: break;
  }
}
