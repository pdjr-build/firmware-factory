# MODCTL

NMEA 2000 (N2K) spudpole control module.

This project implements __MODCTL__, an N2K control interface for one or
two anchor windlasses. The module allows physical switches in one
location on the N2K bus to operate a windlass in some other location
subject to the requirement that the windlass to be operated must have
an N2K interface that supports the [Windlass Network Messages](
https://www.nmea.org/Assets/20190613%20windlass%20amendment,%20128776,%20128777,%20128778.pdf)
protocol.

__MODCTL__ was developed to operate spudpoles, but it will allow
operation of any N2K controlled windlass which uses the referenced
protocol.

Multiple __MODCTL__ modules can be installed on a single bus.

## Physical inputs

__MODCTL__ supports four physical input channels intended for connection
of switch contacts. Inputs are optically-isolated, active high, and rated
for operation at 12 or 24VDC.

| Pin | Windlass | Function | Description                             |
|:----|:---------|:---------|:----------------------------------------|
| 1   | SPUD0    | UP       | Retrieve spudpole 0.                    |
| 2   | SPUD0    | DOWN     | Deploy spudpole 0.                      |
| 3   | SPUD1    | UP       | Retrieve spudpole 1.                    |
| 4   | SPUD1    | DOWN     | Deploy spudpole 1.                      |
| 5   | ---      | GND      | Reference ground for inputs 1..4.       |

Inputs on pins 1 through 4 must be maintained for continuous operation of
the associated spudpole.

## Physical outputs

The module has five zero volt SPST normally-open output channels which
signal the operating state of the controlled spudpole(s) and will
typically be used for connection of external panel indicators. Each
output channel is switched by a reed relay rated at 50VDC 1A maximum
load.

| Pin  | Windlass | Function | Description                            |
|:-----|:---------|:---------|:---------------------------------------|
| 1&2  | ---      | PWR      | Closed when module is powered.  Pulses to indicate that another panel is operating a spudpole. |
| 3&4  | SPUD0    | UP       | Closed when spudpole 0 is docked. Pulses to indicate spudpole 0 is being retrieved. |
| 5&6  | SPUD0    | DOWN     | Closed when spudpole 0 is deployed. Pulses to indicate spudpole 0 is being deployed. |
| 7&8  | SPUD1    | UP       | Closed when spudpole 1 is docked. Pulses to indicate spudpole 1 is being retrieved. |
| 9&10 | SPUD1    | DOWN     | Closed when spudpole 1 is deployed. Pulses to indicate spudpole 1 is being deployed. |

## NMEA bus interface

__MODCTL__ connects to the host N2K bus through a standard M12 5-pin
male connector.

In addition to the usual N2K network management messages the module
accepts the following message types and processes them to drive the
physical outputs.

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 128776 | Anchor Windlass Control Status    | Drives status outputs |
| 128777 | Anchor Windlass Operating Status  | Drives status outputs |

The module issues the following control message types in response to
signals on the physical inputs.

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 126208 | Command Group Function            | 250ms transmition rate|

## Configuring the module

To operate correctly the module must be configured with the NMEA
instance address of the windlass or windlasses which are being
controlled.

The module PCB has an 8-way DIP switch which allows entry of an
instance address in binary and two buttons labelled PRG\_SPUD0 and
PRG\_SPUD1 which are used to associate a remote windlass with each of
the switch operating channels.

To program a channel, the address of the remote windlass interface is
set up on the instance DIP switch and then the appropriate PRG button
must be held closed. After a two second delay, the module LED will
flash once to indicate that the address has been saved to non-volatile
memory. Addresses can be reprogrammed using the same procedure should
that be necessary.

