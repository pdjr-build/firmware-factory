//*********************************************************************
// Windlass.h - class representing a windlass.
//
// Windlass is a windlass state monitor, not a windlass controller: the
// assumption here is that the class will be used to keep track of the
// operating condition of physical device.
//
// 2020 (c) Paul Reeve <preeve@pdjr.eu>
//*********************************************************************

#ifndef WINDLASS_H
#define WINDLASS_H


class Windlass {
  public:
    struct Settings {
      double spoolDiameter;
      double lineDiameter;
      unsigned int turnsPerLayer;
      double usableLineLength;
      double nominalLineSpeed;
      double operatingTime;
      unsigned long (*timerCallback)(int, unsigned long);
    };
    enum OperatingStates { STOPPED, DEPLOYING, RETRIEVING, UNKNOWN };
    Windlass(Settings settings);
    Settings getWindlassSettings();
    void setOperatingState(OperatingStates state);
    OperatingStates getOperatingState();
    void setRotationCount(int rotationCount);
    void incrRotationCount();
    void decrRotationCount();
    void bumpRotationCount();
    int getRotationCount();
    double getDeployedLineLength();
    double getLineSpeed();
    bool isLineFullyDeployed();
    unsigned long getOperatingTime();
  private:
    // PROPERTIES...
    Settings settings;      // Configuration settings
    OperatingStates operatingState;           // Current state of the windlass
    int rotationCount;              // Rotation count
    unsigned long operatingTime;    // Total windlass operating time in seconds
    double lineLengthOnLayer(int layer, int turnsOnLayer);
};

#endif
