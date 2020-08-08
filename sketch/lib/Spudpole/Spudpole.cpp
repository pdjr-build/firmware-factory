/**
 * Spudpole.cpp 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling spudpoles from the manufacturer Ankreo.
 */
 
#include <cstddef>
#include <string.h>
#include "Spudpole.h"

Spudpole::Spudpole(SpudpoleSettings settings) :
  ElectricWindlass((ElectricWindlassSettings) settings) {
  this->settings = settings;
  this->dockedStatus = UNKNOWN;
  this->deployedStatus = UNKNOWN;
}

SpudpoleSettings Spudpole::getSpudpoleSettings() {
  return(this->settings);
}

void Spudpole::setDockedStatus(Spudpole::States state) {
  this->dockedStatus = state;
}

Spudpole::States Spudpole::getDockedStatus() {
  return(this->dockedStatus);
}

void Spudpole::setDeployedStatus(Spudpole::States state) {
  this->deployedStatus = state;
}

Spudpole::States Spudpole::getDeployedStatus() {
  return(this->deployedStatus);
}

bool Spudpole::isDocked() {
  return(this->dockedStatus == Spudpole::YES);
}

bool Spudpole::isWorking() {
  return((this->dockedStatus == NO) && (this->deployedStatus == NO));
}

bool Spudpole::isDeployed() {
  return(this->deployedStatus == YES);
}

