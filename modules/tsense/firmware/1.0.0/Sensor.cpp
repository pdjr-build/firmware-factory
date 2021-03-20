/**********************************************************************
 * Sensor.cpp - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <Sensor.h>

Sensor::Sensor() {
  this->instance = 0xFF;
  this->source = 0xFF;
  this->setPoint = 0;
}

unsigned byte Sensor::getInstance() {
  return(this->instance);
}

unsigned byter Sensor::getSource() {
  return(this->source);
}

int Sensor::getSetPoint() {
  return(this->setPoint);
}

void Sensor::setInstance(unsigned byte instance) {
  this->instance = instance;
}

void Sensor::setSource(unsigned byte source) {
  this->source = source;
}

void Sensor::setSetPoint(int setPoint) {
  this->setPoint = setPoint;
}

void Sensor::invalidate() {
  this->instance = 0xFF;
  this->source = 0xFF;
  this->setPoint = 0;
}
