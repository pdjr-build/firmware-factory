//*********************************************************************
// Windlass.cpp - class representing a windlass.
//
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#include "Windlass.h"

Windlass::Windlass(WindlassSettings settings) {
  this->settings = settings;
  this->state = WindlassStates_UNKNOWN;
  this->rotationCount = 0;
  this->operatingTime = this->settings.operatingTime;
}

WindlassSettings Windlass::getWindlassSettings() {
  return(this->settings);
}

void Windlass::setWindlassState(WindlassStates state) {
  this->state = state;
}

WindlassStates Windlass::getWindlassState() {
  return(this->state);
}

void Windlass::setRotationCount(int rotationCount) {
  this->rotationCount = rotationCount;
}

void Windlass::incrRotationCount() {
  this->rotationCount++;
}

void Windlass::decrRotationCount() {
  this->rotationCount = (this->rotationCount > 0)?(this->rotationCount - 1):0;
}

void Windlass::bumpRotationCount() {
  switch (this->state) {
    case WindlassStates_RETRIEVING:
      this->decrRotationCount();
      break;
    case WindlassStates_DEPLOYING:
      this->incrRotationCount();
      break;
    default:
      break;
  }
}

int Windlass::getRotationCount() {
  return(this->rotationCount);
}

double Windlass::getDeployedLineLength() {
  double retval = 0.0;
  double rlol;
  int layersUsed = (this->rotationCount / this->settings.turnsPerLayer);
  int turnsOnLayer = 0;
  for (int layer = 0; layer <= layersUsed; layer++) {
    turnsOnLayer = (layer < layersUsed)?this->settings.turnsPerLayer:(this->rotationCount % this->settings.turnsPerLayer); 
    rlol = this->lineLengthOnLayer(layer, turnsOnLayer);
    retval += rlol;
  }
  return(retval);
}

double Windlass::getLineSpeed() {
  return(this->settings.nominalLineSpeed);
}

bool Windlass::isLineFullyDeployed() {
  return(this->getDeployedLineLength() > this->settings.usableLineLength);
}

unsigned long Windlass::getOperatingTime() {
  return(this->operatingTime);
}

//*****************************************************************************
// Private methods
//*****************************************************************************

double Windlass::lineLengthOnLayer(int layer, int turnsOnLayer) {
  double retval = turnsOnLayer * (3.1416 * (this->settings.spoolDiameter + this->settings.lineDiameter + (layer * 2 * this->settings.lineDiameter)));
  return(retval);
}
