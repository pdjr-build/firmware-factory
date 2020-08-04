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
typedef struct ElectricWindlassSettings ElectricWindlassSettings;
  
class ElectricWindlass : Windlass {
  public:
    ElectricWindlass(ElectricWindlassSettings settings);
    ElectricWindlassSettings getSettings(); 
    void setControllerVoltage(double voltage);
    double getControllerVoltage();
    bool isControllerUnderVoltage();
    void setMotorCurrent(double current);
    double getMotorCurrent();
    bool isMotorOverCurrent();
  private:
    ElectricWindlassSettings settings;
    double controllerVoltage;
    double motorCurrent;
};

#endif
