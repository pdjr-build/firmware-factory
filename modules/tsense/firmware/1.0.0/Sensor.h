/**********************************************************************
 * Sensor.h - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    Sensor();
    void setIndex(int index);
    void setInstance(unsigned byte instance);
    void setSource(unsigned byte instance);
    void setSetPoint(int setPoint);
    int getIndex();
    unsigned byte getInstance();
    unsigned byte getSource();
    int getSetPoint();
    void invalidate();
  private:
    int index;
    unsigned byte instance;
    unsigned byte source;
    int setPoint;
};

#endif
