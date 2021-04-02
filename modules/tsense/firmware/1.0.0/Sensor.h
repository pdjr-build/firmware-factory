/**********************************************************************
 * Sensor.h - temperature sensor ADT.
 * 2021 (c) Paul Reeve <preeve@pdjr.eu>
 */

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
  public:
    Sensor();
    void setGpio(unsigned char gpio=0);
    void setInstance(unsigned char instance);
    void setSource(unsigned char instance);
    void setSetPoint(double setPoint);
    void setTemperature(double temperature);
    unsigned char getGpio();
    unsigned char getInstance();
    unsigned char getSource();
    double getSetPoint();
    double getTemperature();
    void invalidate(unsigned char gpio);
    void save(int eepromAddress);
    void load(int eepromAddress);
    int getConfigSize();
  private:
    struct Configuration {
      unsigned char gpio;
      unsigned char instance;
      unsigned char source;
      double setPoint;
    };
    struct Sensor::Configuration config;
    double temperature;
};

#endif
