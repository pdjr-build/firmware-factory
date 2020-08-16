# MODCTL

NMEA 2000 (N2K) spudpole control module.

This project implements __MODCTL__, an N2K control interface for one or
two anchor windlasses. The module allows physical switches in one
location to operate a windlass in some other location subject to the
requirement that the windlass to be operated must have an N2K interface
that supports the N2K
[Windlass Network Messages control protocol](
https://www.nmea.org/Assets/20190613%20windlass%20amendment,%20128776,%20128777,%20128778.pdf)

Although __MODCTL__ was developed to support spudpole operation, it
will allow operation of any N2K controlled windlass which uses the
described protocol.

## Switch inputs

__MODCTL__ supports four input channels which allow connection of
switches to operate a maximum of two spudpoles, SPUD0 and SPUD1. Switch
inputs are optically-isolated, active high, and rated for 12/24VDC and
connections are made through a five way terminal block.

1. SPUD0\_UP;
2. SPUD0\_DOWN;
3. SPUD1\_UP;
4. SPUD1\_DOWN;
5. GND.

## Status outputs

The module has six zero volt output channels which signal the operating
state of the controlled spudpole(s). Each output channel is rated at
50VDC 1A maximum load and will typically be used for connection of an
external panel indicator.

6. SPUD0\_DOCKED 
7. SPUD0\_DOCKED
8. SPUD0\_DEPLOYED
9. SPUD0\_DEPLOYED
10. SPUD0\_OPERATING
11. SPUD0\_OPERATING
12. SPUD1\_DOCKED 
13. SPUD1\_DOCKED
14. SPUD1\_DEPLOYED
15. SPUD1\_DEPLOYED
16. SPUD1\_OPERATING
17. SPUD1\_OPERATING

## NMEA bus interface

__MODCTL__ connects to the host N2K bus through a standard M12 5-pin
male connector.

In addition to the usual N2K network management messages the module
accepts the following message types:

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 128777 | Anchor Windlass Operating Status  | Drives status outputs |

And issues the following control message types when any of the input
channels are high (on).

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 126208 | Command Group Function            | 250ms transmition rate|

## Configuring the module

To operate correctly the module must be configured with the NMEA
instance addresses of the windlasses which are being controlled.

The module PCB has an 8-way DIP switch which allows entry of an
instance address in binary and two buttons labelled PRG\_SPUD0 and
PRG\_SPUD1 which are used to associate a remote windlass with each of
the switch operating channels.

To program a channel, the address of the remote windlass interface is
set up on the instance DIP switch and then the appropriate PRG button
must be held closed. After a two second delay, the module LED will
flash once to indicate that the address has been saved to non-volatile
memory. Addresses can be reprogrammed using the same procedure at any
time.

