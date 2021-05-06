/**********************************************************************
 * PGN128007.h (c) 2021 Paul Reeve <preeve@pdjr.eu>
 */

#ifndef _PGN128007_H_
#define _PGN128007_H_

#define PGN128007_FieldCount 6

#define PGN128007_ThrusterIdentifier_FieldIndex 1
#define PGN128007_ThrusterMotorType_FieldIndex 2
#define PGN128007_MotorPowerRating_FieldIndex 4
#define PGN128007_MaximumMotorTemperatureRating_FieldIndex 5
#define PGN128007_MaximumRotationalSpeed_FieldIndex 6

#define PGN128007_StaticUpdateInterval 0
#define PGN128007_DynamicUpdateInterval 0

struct PGN128007_Properties {
  unsigned char ThrusterIdentifier;
  tN2kDD487 ThrusterMotorType;
  int MotorPowerRating;
  double MaximumMotorTemperatureRating;
  double MaximumRotationalSpeed;
};

union PGN128007_GenericField {
  unsigned char F01;
  tN2kDD487 F02;
  int F04;
  double F05;
  double F06;
};

struct PGN128007_UpdateField {
  bool modified;
  PGN128007_GenericField value;
};

class PGN128007 {
  public:
    PGN128007();
    PGN128007_Properties getProperties();
    unsigned char getThrusterIdentifier();
    tN2kDD487 getThrusterMotorType();
    int getMotorPowerRating();
    double getMaximumMotorTemperatureRating();
    double getMaximumRotationalSpeed();
    PGN128007_GenericField getField(int index);
    void setProperties(PGN128007_Properties value);
    void setThrusterIdentifier(unsigned char value);
    void setThrusterMotorType(tN2kDD487 value);
    void setMotorPowerRating(int value);
    void setMaximumMotorTemperatureRating(double value);
    void setMaximumRotationalSpeed(double value);
    void setField(int index, PGN128007_GenericField value);
  private:
    PGN128007_Properties properties;
};

#endif