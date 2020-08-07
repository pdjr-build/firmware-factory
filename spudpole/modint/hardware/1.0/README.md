# MODINT v1.0

This module design includes [circuit schematic](schematic.pdf), [pcb layout](pcb.pdf)
and [prototype enclosure](enclosure.pdf) proposals for an NMEA 2000 compatible
spudpole interface built around a Teensy 3.2 MCU. The module is designed to be
powered directly from the host NMEA 2000 bus.

## Packaging proposal

The protoype PCB design is configured for an 80x80x23 flanged ABS
[enclosure](https://docs.rs-online.com/960c/0900766b814af9a1.pdf)
with grommeted cable entry for sensor connections and an industry
standard M12 5-pin industrial connector for NMEA/CAN connection.
The enclosure is drilled to expose PCB mounted status LEDs.

## Circuit design

Module power at 5VDC is derived from the NMEA bus via a DC-DC converter.
The module draws approximately 150mA from the bus power supply giving an
NMEA LEN of 3. The NMEA bus is protected by a self-resetting 1A fuse.

The module's NMEA/CAN data interface is implemented using an MCP2551
industry standard transceiver and includes a switchable 120-ohm bus
termination resistor.

Four otically isolated sensor inputs rated at 12/24VDC allow connection
of MOTOR-RETRIEVING, MOTOR-DEPLOYING, DOCKED and DEPLOYED sensors and a
5VDC powered interface supports connection of an induction proximity
sensor for use as a windlass rotation counter.

The module is able to directly control a windlass motor through UP and
DOWN relay outputs which are driven by control messages received over
the host NMEA bus.

An installer can quickly assign the module an instance address using a
DIP swith and LED indicators show relay state, power and data
transmission status. 

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


