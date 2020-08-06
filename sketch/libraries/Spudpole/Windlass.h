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

struct WindlassSettings {
  double spoolDiameter;
  double lineDiameter;
  unsigned int turnsPerLayer;
  double usableLineLength;
  double nominalLineSpeed;
  double operatingTime;
  unsigned long (*timerCallback)(int, unsigned long);
};
typedef struct WindlassSettings WindlassSettings;

enum WindlassStates {
  WindlassStates_STOPPED = 0,
  WindlassStates_DEPLOYING = 1,
  WindlassStates_RETRIEVING = 2,
  WindlassStates_UNKNOWN = 3
};
  
class Windlass {
  public:
    Windlass(WindlassSettings settings);
    WindlassSettings getSettings();
    void setState(WindlassStates state);
    WindlassStates getState();
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
    WindlassSettings settings;      // Configuration settings
    WindlassStates state;           // Current state of the windlass
    int rotationCount;              // Rotation count
    unsigned long operatingTime;    // Total windlass operating time in seconds
    double lineLengthOnLayer(int layer, int turnsOnLayer);
};

#endif
