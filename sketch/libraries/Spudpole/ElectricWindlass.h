//*********************************************************************
// ElectricWindlass.h - class representing an electric windlass.
//
// Just like its parent class, ElectricWindlass is a windlass state
// monitor, not a windlass controller: the assumption here is that the
// class will be used to keep track of the operating condition of
// physical device.
//
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#ifndef ELECTRICWINDLASS_H
#define ELECTRICWINDLASS_H

#include "Windlass.h"

struct ElectricWindlassSettings {
  WindlassSettings windlassSettings;
  double nominalControllerVoltage;
  double nominalMotorCurrent;
};

enum ElectricWindlassStates {
  ElectricWindlassStates_STOPPED = 0,
  ElectricWindlassStates_RETRIEVING = 1,
  ElectricWindlassStates_DEPLOYING = 2,
  ElectricWindlassStates_UNKNOWN
};
  
  
class ElectricWindlass : public Windlass {
  public:
    ElectricWindlass(ElectricWindlassSettings settings);
    ElectricWindlassSettings getElectricWindlassSettings();
    void setElectricWindlassState(ElectricWindlassStates state);
    ElectricWindlassStates getElectricWindlassState(); 
    void setControllerVoltage(double voltage);
    double getControllerVoltage();
    bool isControllerUnderVoltage();
    void setMotorCurrent(double current);
    double getMotorCurrent();
    bool isMotorOverCurrent();
  private:
    ElectricWindlassSettings settings;
    ElectricWindlassStates state;
    double controllerVoltage;
    double motorCurrent;
};

#endif
