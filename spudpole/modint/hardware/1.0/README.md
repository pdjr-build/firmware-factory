# MODINT v1.0

Prototype hardware design including 
[schematic](schematic.pdf)
and
[pcb](pcb.pdf)
for the __MODINT__ spudpole interface.

__MODINT__ is built around a Teensy 3.2 MCU. The module is powered directly
from the NMEA 2000 bus via an isolated 2A DC-DC converter.

NMEA data signals are interfaced through a MCP2551 industry standard CAN
transceiver.  The module accepts 12/24VDC inputs from DOCKED and DOWN sensors
interfaced through 4N25 optical isolators. A rotation SENSOR interface is
powered by the module at 5VDC, diode protected and current limited by a
resettable fuse.

UP and DOWN relay outputs provide isolated control signalling for direct
connection to a windlass.

Status LEDs indicate operation of the relay outputs and a single bi-color
LED indicates power and NMEA transmit.

## Inputs

The INSTANCE dip switch allows setting of the module's NMEA instance address.

* INSTANCE(1): Address bit 0
* INSTANCE(2): Address bit 1
* INSTANCE(3): Address bit 2
* INSTANCE(4): Address bit 3
* INSTANCE(5): Address bit 4
* INSTANCE(6): Address bit 5
* INSTANCE(7): Address bit 6
* INSTANCE(8): Address bit 7

The SENSOR connector supports connection of a simple proximity sensor for
counting winch spool rotations.

* SENSOR(1): GND
* SENSOR(2): +5VDC
* SENSOR(3): Sensor input

The LIMIT connector supports opto-isolated connection of 12/24VDC signals from
existing spudpole sensors.

* LIMIT(1): Docked / Stopped sensor GND
* LIMIT(2): Docked sensor +ve
* LIMIT(3): Stopped sensor +ve

## Outputs

The module generates two outputs via isolated relays on the RELAY connector:

* RELAY(1): DOWN relay common
* RELAY(2): DOWN relay switched
* RELAY(3): UP relay common
* RELAY(4): UP relay switched


