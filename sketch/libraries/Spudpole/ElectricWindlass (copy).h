//*********************************************************************
// ElectricWindlass.h - class representing an electric windlass.
//
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#ifndef ELECTRICWINDLASS_H
#define ELECTRICWINDLASS_H

#include "Windlass.h"

struct ElectricWindlassSettings {
  WindlassSettings windlassSettings;
  void (*controlCallback)(int);
  double nominalControllerVoltage;
  double nominalMotorCurrent;
};
typedef struct ElectricWindlassSettings ElectricWindlassSettings;
  
class ElectricWindlass : Windlass {
  public:
    ElectricWindlass(ElectricWindlassSettings settings);
    ElectricWindlassSettings getSettings(); 
    void deploy();
    void retrieve();
    void stop();
    void setControllerVoltage(double voltage);
    double getControllerVoltage();
    bool isControllerUnderVoltage();
    void setMotorCurrent(double current);
    double getMotorCurrent();
    bool isMotorOverCurrent();
  private:
    ElectricWindlassSettings settings;
    void (*controlCallback)(int);
    double controllerVoltage;
    double motorCurrent;
};

#endif
