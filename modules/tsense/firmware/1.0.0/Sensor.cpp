/**********************************************************************
 * Sensor.cpp - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <Sensor.h>

Sensor::Sensor() {
  this->gpio = 0;
  this->instance = 0xFF;
  this->source = 0xFF;
  this->setPoint = 0;
  this->temperature = 0.0;
}

unsigned byte Sensor::getGpio() {
  return(this->gpio);
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

float Sensor::getTemperature() {
  return(this->temperature);
}

void Sensor::setGpio(unsigned byte gpio) {
  this->gpio = gpio;
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

void Sensor::setTemperature(float temperature) {
  this.temperature = temperature;
}

void Sensor::invalidate(unsigned byte gpio) {
  this->gpio = gpio;
  this->instance = 0xFF;
  this->source = 0xFF;
  this->setPoint = 0;
  this->temperature = 0.0;
}
