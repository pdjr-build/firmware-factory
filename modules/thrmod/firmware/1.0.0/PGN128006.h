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

struct PGN128006_Field {
  bool modified;
  union {
    uint8_t F02;
    tN2kDD473 F03;
    tN2kDD002 F04;
    tN2kDD474 F05;
    uint8_t F06;
    tN2kDD475 F07;
    double F08;
    double F09;
  };
};

#endif