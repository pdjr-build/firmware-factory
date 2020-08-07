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
  this->dockedStatus = SpudpoleStates_UNKNOWN;
  this->deployedStatus = SpudpoleStates_UNKNOWN;
}

SpudpoleSettings Spudpole::getSpudpoleSettings() {
  return(this->settings);
}

void Spudpole::setDockedStatus(SpudpoleStates state) {
  this->dockedStatus = state;
}

SpudpoleStates Spudpole::getDockedStatus() {
  return(this->dockedStatus);
}

void Spudpole::setDeployedStatus(SpudpoleStates state) {
  this->deployedStatus = state;
}

SpudpoleStates Spudpole::getDeployedStatus() {
  return(this->deployedStatus);
}

bool Spudpole::isDocked() {
  return(this->dockedStatus == SpudpoleStates_YES);
}

bool Spudpole::isWorking() {
  return((this->dockedStatus == SpudpoleStates_NO) && (this->deployedStatus == SpudpoleStates_NO));
}

bool Spudpole::isDeployed() {
  return(this->deployedStatus == SpudpoleStates_YES);
}

