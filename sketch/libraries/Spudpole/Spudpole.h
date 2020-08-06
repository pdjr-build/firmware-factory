//*************************************************************************
// Spudpole.h 20 July 2020 (c) <preeve@pdjr.eu>
//
// Abstract data type modelling a spudpole.
//
// Just like its parent class, Spudpole is a state monitor - in this case
// for a windlass operated pole anchor. The assumption here is that the
// class will be used to keep track of the operating condition of some
// physical device.
//
// Spudpole adds two properties to ElectricWindlass:
//
// docked represents the docking state of the spudpole. Typically this is
// reported by a sensor which signals when the spudpole is fully retracted
// and stowed.
//
// fullyDeployed represents the condition when the spudpole has been
// deployed and has been arrested: either by reaching by its rode or by
// something external to the system (like the seabed). Typically this is
// reported by a sensor which detects in some way when the windlass load
// falls to zero.
//
//*************************************************************************
 
#ifndef SPUDPOLE_H
#define SPUDPOLE_H

#include "ElectricWindlass.h"

enum SpudpoleStates {
  SpudpoleStates_NO = 0,
  SpudpoleStates_YES = 1,
  SpudpoleStates_UNKNOWN = 3
};

typedef struct ElectricWindlassSettings SpudpoleSettings;
  
class Spudpole : public ElectricWindlass {
  public:
    Spudpole(SpudpoleSettings settings);
    SpudpoleSettings getSettings();
    void setDockedStatus(SpudpoleStates state);
    SpudpoleStates getDockedStatus();
    void setFullyDeployedStatus(SpudpoleStates state);
    SpudpoleStates getFullyDeployedStatus();
    bool isDocked();
    bool isWorking();
    bool isFullyDeployed();
  private:
    SpudpoleSettings settings;
    SpudpoleStates dockedStatus;
    SpudpoleStates fullyDeployedStatus;
};

#endif
