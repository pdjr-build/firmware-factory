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


class ElectricWindlass : public Windlass {
  public:
    struct Settings {
      Windlass::Settings windlassSettings;
      double nominalControllerVoltage;
      double nominalMotorCurrent;
    };
    ElectricWindlass(ElectricWindlass::Settings settings);
    ElectricWindlass::Settings getElectricWindlassSettings();
    void setControllerVoltage(double voltage);
    double getControllerVoltage();
    bool isControllerUnderVoltage();
    void setMotorCurrent(double current);
    double getMotorCurrent();
    bool isMotorOverCurrent();
  private:
    Settings settings;
    double controllerVoltage;
    double motorCurrent;
};

#endif
