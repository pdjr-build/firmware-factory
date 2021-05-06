/**********************************************************************
 * PGN128008.h (c) 2021 Paul Reeve <preeve@pdjr.eu>
 */

#ifndef _PGN128008_H_
#define _PGN128008_H_

#define PGN128008_FieldCount 6

#define PGN128008_ThrusterIdentifier_FieldIndex 2
#define PGN128008_ThrusterMotorEvents_FieldIndex 3
#define PGN128008_MotorCurrent_FieldIndex 4
#define PGN128008_MotorTemperature_FieldIndex 5
#define PGN128008_TotalMotorOperatingTime_FieldIndex 6

#define PGN128008_StaticUpdateInterval 5000
#define PGN128008_DynamicUpdateInterval 500

struct PGN128008_Properties {
  unsigned char ThrusterIdentifier;
  tN2kDD471 ThrusterMotorEvents;
  unsigned char MotorCurrent;
  double MotorTemperature;
  unsigned long TotalMotorOperatingTime;
};

union PGN128008_GenericField {
  unsigned char F02;
  tN2kDD471 F03;
  unsigned char F04;
  double F05;
  unsigned long F06;
};

struct PGN128008_UpdateField {
  bool modified;
  PGN128008_GenericField value;
};

class PGN128008 {
  public:
    PGN128008();
    PGN128008_Properties getProperties();
    unsigned char getThrusterIdentifier();
    tN2kDD471 getThrusterMotorEvents();
    unsigned char getMotorCurrent();
    double getMotorTemperature();
    unsigned long getTotalMotorOperatingTime();
    PGN128008_GenericField getField(int index);
    void setProperties(PGN128008_Properties value);
    void setThrusterIdentifier(unsigned char value);
    void setThrusterMotorEvents(tN2kDD471 value);
    void setMotorCurrent(unsigned char value);
    void setMotorTemperature(double value);
    void setTotalMotorOperatingTime(unsigned long value);
    void setField(int index, PGN128008_GenericField value);
    void bumpTotalMotorOperatingTime(unsigned long value);
  private:
    PGN128008_Properties properties;
};

#endif