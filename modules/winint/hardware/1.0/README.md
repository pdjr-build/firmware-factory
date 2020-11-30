# WININT - windlass interface module (1.0)

[Circuit schematic](schematic.pdf)\
[PCB layout](pcb.pdf)

for an NMEA 2000 compatible spudpole interface
built around a Teensy 3.2 MCU.

The interface is designed to be powered directly from the host NMEA
2000 bus and has full electrical isolation of all external inputs
and outputs.

## PCB and packaging proposal

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

The module interfaces to the host CAN bus through an
[MCP2551 high-speed CAN transceiver](https://docs.rs-online.com/f763/0900766b8140ba57.pdf).
The CAN H and L signals are cleaned through simple RC input
filtering.
A switch on the module allows the installer to select whether
or not to connect the CAN screen to the module ground plane.

### Sensor inputs 

Six optically isolated screw connector inputs rated at 12/24VDC
allow connection of up to six physical sensors.
All sensor inputs are active high.

Three sensor inputs (SENSOR_ROT, SENSOR_RTD and SENSOR_DPD) are
essential for module operation, whilst the other three inputs allow
improved fault detection and refined control.

#### SENSOR_ROT
This input should be fed from a rotation sensor directly monitoring the
windlass spool and should receive one pulse per spool revolution.

#### SENSOR_RTD
This input should be high when the ground tackle operated by the windlass
is fully ReTrieveD and / or docked.

#### SENSOR_DPD
This input should be high when the ground tackle is fully DePloyeD.

#### SENSOR_RTG
This input should be high when the windlass is ReTrievinG its cable.
On an electrically operated windlass this signal could come directly from the
windlass actuator.

#### SENSOR_DPG
This input should be high when the windlass is DePloyinG its cable.
On an electrically operated windlass this signal could come directly from the
windlass actuator.

#### SENSOR_OVL
This input should be high when the windlass is experiencing or about to
experience an overload condition (for example, when the ground tackle
is partially fouled).
This signal could come from a motor current sensor or some form of strain
sensor monitoring the physical structure of the windlass installation.

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

| Switch   | Pole | Description |
|:---------|:----:|:------------|
| CONFIG   | 1    | Close to enable relay output. With this switch open, all aspects of the module (including status reports and LED outputs) function normally except that the output relays are not operated. |
| CONFIG   | 2    | Close to connect NMEA cable shield to module ground. |
| INSTANCE | 1    | NMEA instance number bit 0. |
| INSTANCE | 2    | NMEA instance number bit 1. |
| INSTANCE | 3    | NMEA instance number bit 2. |
| INSTANCE | 4    | NMEA instance number bit 3. |
| INSTANCE | 5    | NMEA instance number bit 4. |
| INSTANCE | 6    | NMEA instance number bit 5. |
| INSTANCE | 7    | NMEA instance number bit 6. |

### LED indicators

The module has six LED indicators: one on the MCU, two adjacent to
the output relays and three status LEDs (PWR, UP and DOWN) which
are visible from outside the module enclosure.

| LED        | Description | 
|:-----------|:------------|
| MCU        | No module specific use (see MCU documentation). |
| UP relay   | Illuminates when the UP relay NO contact is closed. |
| DOWN relay | Illuminates when the DOWN relay NO contact is closed. |
| PWR        | Shows power state and is modulated when NMEA communication is in progress. | 
| UP         | Modulated to reflect the module operating state. |
| DOWN       | Modulated to reflect the module operating state. | 
