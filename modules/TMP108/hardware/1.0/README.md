# TSENSE 1.0 Prototype Hardware

This microcontroller-based design uses just a handful of components
to implement an 8-channel NMEA 2000 temperature sensor module which
is powered from the host NMEA 2000 bus and draws around 50mA of
current at a nominal 12VDC with all eight sensor channels in use.
This equates to an NMEA bus load of less that 1 LEN.

The module supports
[National Semiconductor LM335Z](https://www.switchelectronics.co.uk/pub/media/pdf/LM335Z.pdf)
precision temperature sensors connected through a two-wire interface.
The module cannot be used with resistance or current-loop sensors.

The module has a simple user configuration interface consisting of a
DIP switch, momentary push-button and LED outputs.
A discrete LED indicates module operating state.

## Active components

| Element         | Device |
|:----------------|:-------|
| MCU             | [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) |
| Power supply    | [Traco Power TMR 1-1211](https://tracopower.com/tmr1-datasheet/) |
| CAN transceiver | [MicroChip MCP2551](http://ww1.microchip.com/downloads/en/devicedoc/21667e.pdf) |

## Schematic design

<a href="schematic-1.0.pdf">
<img align="right" width="400" src="schematic-1.0.svg">
</a>
<br clear="both"/>

## Physical design

The prototype module uses through-hole components on a 75mm square PCB
mounted in a flanged ABS enclosure.
The enclosure is machined to accommodate an NMEA cable connector, two
6mm cable grommets and single 3mm LED indicator.

| Element         | Device |
|:----------------|:-------|
| Enclosure       | [RS 919-0357](https://docs.rs-online.com/960c/0900766b814af9a1.pdf) |
| NMEA connector  | [M12 5-pin connector](https://docs.rs-online.com/e3ad/0900766b8152901f.pdf) |

<a href="pcb-1.0.pdf">
<img align="right" width="400" src="pcb-1.0.svg">
</a>
<br clear="both"/>
