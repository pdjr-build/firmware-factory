/**********************************************************************
 * Sensor.h - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    Sensor();
    void setGpio(unsigned byte gpio=0);
    void setInstance(unsigned byte instance);
    void setSource(unsigned byte instance);
    void setSetPoint(int setPoint);
    void setTemperature(float temperature);
    unsigned byte getGpio();
    unsigned byte getInstance();
    unsigned byte getSource();
    int getSetPoint();
    float getTemperature();
    void invalidate(unsigned byte gpio);
  private:
    unsigned byte gpio;
    unsigned byte instance;
    unsigned byte source;
    int setPoint;
    float temperature;
};

#endif
