# MODINT v1.0

This hardware design includes [schematic](schematic.pdf), [pcb](pcb.pdf) and engineering
designs for an NMEA 2000 compatible spudpole interface built around the Teensy 3.2 MCU.

The module is powered directly from the NMEA 2000 bus via an isolated DC-DC converter and
has a nominal LEN of 3. The module's NMEA/CAN data interface is implemented using an MCP2551
industry standard transceiver.  Physical bus connection is by an industry standard M12
5-pin industrial connector and the module includes a switchable 120-ohm bus termination
resistor.

Four otically isolated sensor inputs rated at 12/24VDC allow connection of MOTOR-UP, MOTOR-DOWN,
DOCKED and DEPLOYED sensors; whilst a 5VDC powered interface supports connection of an inductive
windlass rotation counter;

The module is able to directly control a windlass motor through UP and DOWN relay outputs
which respond to control messages received over the host NMEA bus.

An installer can quickly assign the module an instance address using a DIP swith and LED
indicators show relay state, power and data status. 

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


