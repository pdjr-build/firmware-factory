/**********************************************************************
 * Sensor.h - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    Sensor();
    void setInstance(unsigned byte instance);
    void setSource(unsigned byte instance);
    void setSetPoint(int setPoint);
    unsigned byte getInstance();
    unsigned byte getSource();
    int getSetPoint();
    void invalidate();
  private:
    unsigned byte instance;
    unsigned byte source;
    int setPoint;
};

#endif
