/**********************************************************************
 * PGN128006.h (c) 2021 Paul Reeve <preeve@pdjr.eu>
 */

#ifndef _PGN128006_H_
#define _PGN128006_H_

#define PGN128006_FieldCount 9

#define PGN128006_ThrusterIdentifier_FieldIndex 2
#define PGN128006_ThrusterDirectionControl_FieldIndex 3
#define PGN128006_PowerEnable_FieldIndex 4
#define PGN128006_ThrusterRetractControl_FieldIndex 5
#define PGN128006_SpeedControl_FieldIndex 6
#define PGN128006_ThrusterControlEvents_FieldIndex 7
#define PGN128006_CommandTimeout_FieldIndex 8
#define PGN128006_AzimuthControl_FieldIndex 9

#define PGN128006_StaticUpdateInterval 5000
#define PGN128006_DynamicUpdateInterval 500

struct PGN128006_Properties {
  uint8_t ThrusterIdentifier;
  tN2kDD473 ThrusterDirectionControl;
  tN2kDD002 PowerEnable;
  tN2kDD474 ThrusterRetractControl;
  uint8_t SpeedControl;
  tN2kDD475 ThrusterControlEvents;
  double CommandTimeout;
  double AzimuthControl;
};

union PGN128006_GenericField {
  uint8_t F02;
  tN2kDD473 F03;
  tN2kDD002 F04;
  tN2kDD474 F05;
  uint8_t F06;
  tN2kDD475 F07;
  double F08;
  double F09;
};

struct PGN128006_UpdateField {
  bool modified;
  PGN128006_GenericField value;
};

class PGN128006 {
  public:
    PGN128006();
    PGN128006_Properties getProperties();
    uint8_t getThrusterIdentifier();
    tN2kDD473 getThrusterDirectionControl();
    tN2kDD002 getPowerEnable();
    tN2kDD474 getThrusterRetractControl();
    uint8_t getSpeedControl();
    tN2kDD475 getThrusterControlEvents();
    double getCommandTimeout();
    double getAzimuthControl();
    PGN128006_GenericField getField(int index);
    void setProperties(PGN128006_Properties value);
    void setThrusterIdentifier(uint8_t value);
    void setThrusterDirectionControl(tN2kDD473 value);
    void setPowerEnable(tN2kDD002 value);
    void setThrusterRetractControl(tN2kDD474 value);
    void setSpeedControl(uint8_t value);
    void setThrusterControlEvents(tN2kDD475 value);
    void setCommandTimeout(double value);
    void setAzimuthControl(double value);
    void setField(int index, PGN128006_GenericField value);
  private:
    PGN128006_Properties properties;
};

#endif