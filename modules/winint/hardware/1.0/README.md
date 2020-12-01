# WININT - windlass interface module (1.0)

## Circuit design

<a href="schematic.pdf">
<img align="right" width="400" src="schematic.svg">
</a>

The module circuit is based on a
[Teensy 3.2](https://www.pjrc.com/store/teensy32.html)
MCU.

Power is drawn directly from the host NMEA 2000 bus via a
[TEC2-2411WI](https://www.tracopower.com/products/tec2wi.pdf)
DC-DC converter delivering 5VDC at maximumum 2A.
The NMEA power tap is fused at 500mA.

The module interfaces to the host bus through an
[MCP2551](https://docs.rs-online.com/f763/0900766b8140ba57.pdf)
high-speed CAN transceiver.
The CAN H and L signals are cleaned through simple RC input
filtering.
A switch allows the installer to select whether or not to
connect the host bus screen to the module ground plane.

Six active-high sensor input channels will accept signals at either
12VDC or 24VDC.
Each input channel is protected by a
[PC817](http://www.soselectronic.cz/a_info/resource/d/pc817.pdf)
opto-isolator.

Two zero-volt SPDT relay output channels are provided for the
connection of UP and DOWN actuators.
The CO, NO and NC contacts are available to the installer and the
NO contact on each relay is snubber protected against damage by
inductive loads making the relays suitable for driving heavy duty relay
coils and solenoids.

The module is configured through DIP switch setting and a range of LED
outputs offer status information.

### Sensor input channels

| Channel | Required | Input characteristic |
|:--------|:---------|:---------------------|
| ROT     | Yes      | One high pulse per windlass spool revolution. |
| RTD     | Yes      | High when the ground tackle operated by the windlass is fully retrieved. |
| DPD     | Yes      | High when the ground tackle is fully deployed. |
| RTG     | No       | High when the windlass is retrieving its cable. |
| DPG     | No       | High when the windlass is deploying its cable. |
| OVL     | No       | High when the windlass is experiencing or about to experience an overload condition |

### Relay output channels

### Switch inputs

| Switch   | Pole | Description |
|:---------|:----:|:------------|
| CONFIG   | 1    | Close to enable relay output. |
| CONFIG   | 2    | Close to connect NMEA cable shield to module ground. |
| INSTANCE | 1    | NMEA instance number bit 0. |
| INSTANCE | 2    | NMEA instance number bit 1. |
| INSTANCE | 3    | NMEA instance number bit 2. |
| INSTANCE | 4    | NMEA instance number bit 3. |
| INSTANCE | 5    | NMEA instance number bit 4. |
| INSTANCE | 6    | NMEA instance number bit 5. |
| INSTANCE | 7    | NMEA instance number bit 6. |

### Indicator outputs

| LED        | Description | 
|:-----------|:------------|
| MCU        | No module specific use (see MCU documentation). |
| UP relay   | Illuminates when the UP relay NO contact is closed. |
| DOWN relay | Illuminates when the DOWN relay NO contact is closed. |
| PWR        | Shows power state and is modulated when NMEA operating commands are being received. | 
| UP         | Modulated to reflect the windlass retrieve state. Modulated in concert with DOWN to indicate overload condition. |
| DOWN       | Modulated to reflect the windlass deploy state. Modulated in concert with UP to indicate overload condition. | 

### PCB and packaging proposal

This first-generation design exclusively uses through-hole components
and is implemented as an 80mm by 80mm x 1.6mm two-layer 1oz copper PCB.

The PCB is installed in a 25mm deep, flanged, ABS
[enclosure](https://docs.rs-online.com/960c/0900766b814af9a1.pdf)
with grommeted cable entry for sensor connections and an industry
standard female
[M12 5-pin connector](https://docs.rs-online.com/e3ad/0900766b8152901f.pdf)
for NMEA bus connection.

The enclosure is drilled to expose PCB mounted status LEDs.

## Circuit design

### MCU

[Teensy 3.2](https://www.pjrc.com/store/teensy32.html)

### Power supply

The module is powered directly from 12VDC NMEA 2000 power
on CAN lines S and C.
An integrated DC-DC converter provides a regulated 5VDC 2A
supply for all board components and will tolerate input
voltage in the range 9VDC through 32VDC.

The module draws approximately 100mA from the NMEA bus giving
an NMEA LEN of 1.
The NMEA bus is protected by a self-resetting 500mA polyfuse.

### CAN data interface


### Sensor inputs 

Six optically isolated screw connector inputs rated at 12/24VDC
allow connection of up to six physical sensors.
All sensor inputs are active high.

Three sensor inputs (SENSOR_ROT, SENSOR_RTD and SENSOR_DPD) are
essential for module operation, whilst the other three inputs allow
improved fault detection and refined control.


### Relay outputs

The module provides two zero-volt SPDT relay outputs for connection of
UP and DOWN actuators.
The relays are rated at 50VDC 2A and CO, NO and NC contacts are
available through screw connection terminals.
The NO contact on each relay is snubber protected against damage by
inductive loads making the relays suitable for driving heavy duty relay
coils and solenoids.

### PCB switches

The PCB has two DIL switch modules CONFIG and INSTANCE.


