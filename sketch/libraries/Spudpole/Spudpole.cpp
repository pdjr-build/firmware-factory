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
  this->fullyDeployedStatus = SpudpoleStates_UNKNOWN;
}

SpudpoleSettings Spudpole::getSettings() {
  return(this->settings);
}

void Spudpole::setDockedStatus(SpudpoleStates state) {
  this->dockedStatus = state;
}

SpudpoleStates Spudpole::getDockedStatus() {
  return(this->dockedStatus);
}

void Spudpole::setFullyDeployedStatus(SpudpoleStates state) {
  this->fullyDeployedStatus = state;
}

SpudpoleStates Spudpole::getFullyDeployedStatus() {
  return(this->fullyDeployedStatus);
}

bool Spudpole::isDocked() {
  return(this->dockedStatus == SpudpoleStates_YES);
}

bool Spudpole::isWorking() {
  return((this->dockedStatus == SpudpoleStates_NO) && (this->fullyDeployedStatus == SpudpoleStates_NO));
}

bool Spudpole::isFullyDeployed() {
  return(this->fullyDeployedStatus == SpudpoleStates_YES);
}

