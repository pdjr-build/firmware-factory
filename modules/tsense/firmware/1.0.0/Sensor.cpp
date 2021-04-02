/**********************************************************************
 * Sensor.cpp - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#include <cstddef>
#include <EEPROM.h>
#include <Sensor.h>

Sensor::Sensor() {
  this->config = { 0x00, 0xff, 0xff, 0.0 };
  this->temperature = 0.0;
}

unsigned char Sensor::getGpio() {
  return(this->config.gpio);
}

unsigned char Sensor::getInstance() {
  return(this->config.instance);
}

unsigned char Sensor::getSource() {
  return(this->config.source);
}

double Sensor::getSetPoint() {
  return(this->config.setPoint);
}

double Sensor::getTemperature() {
  return(this->temperature);
}

void Sensor::setGpio(unsigned char gpio) {
  this->config.gpio = gpio;
}

void Sensor::setInstance(unsigned char instance) {
  this->config.instance = instance;
}

void Sensor::setSource(unsigned char source) {
  this->config.source = source;
}

void Sensor::setSetPoint(double setPoint) {
  this->config.setPoint = setPoint;
}

void Sensor::setTemperature(double temperature) {
  this->temperature = temperature;
}

void Sensor::invalidate(unsigned char gpio) {
  this->config = { 0x00, 0xff, 0xff, 0.0 };
  this->temperature = 0.0;
}

void Sensor::save(int eepromAddress) {
  EEPROM.put(eepromAddress, this->config);
}

void Sensor::load(int eepromAddress) {
  EEPROM.get(eepromAddress, this->config);
}

int Sensor::getConfigSize() {
  return(sizeof this->config);
}
