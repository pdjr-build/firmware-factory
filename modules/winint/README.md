# WININT - NMEA 2000 windlass interface module

__WININT__ is an NMEA 2000 module which implements a windlass operating
interface.
The module was developed to support pole anchors built by the manufacturer
ANKREO, but it may well be useful with other windlass-based hardware that
has similar control and monitoring requirements.
The module uses the N2K Windlass Network Messages protocol to receive
commands from a remote windlass controller and to transmit reports on the
current hardware state.

__WININT__ connects to the NMEA bus by a standard M12 5-pin circular
connector and is powered directly from the NMEA bus.
The module has an NMEA LEN of 1.5.

Two control output channels, UP and DOWN are implemented as volt free
relays rated at 5A suitable for winch relay or solenoid operation.
Six sensor input channels can be connected to 12/24V external signals
or to solid state proximity NPN proximity sensors.
Power for each sensor channel can be supplied by the module.
Diagnostic LEDS confirm NMEA connection and module operating status.
The module is configured by a PCB mounted DIP switch which allows entry
of an NMEA instance address that defines the windlass installation and
allows multiple __WININT__ modules to co-exist on the same network.

## About the module


This project implements __MODINT__, an N2K interface for spudpoles which uses
the Windlass Network Messages interface described in this
[Technical Bulletin(
https://www.nmea.org/Assets/20190613%20windlass%20amendment,%20128776,%20128777,%20128778.pdf)


__MODINT__ accepts physical inputs from a range of spudpole sensors and
reports the current spudpole state over N2K.

The module also accepts spudpole operating commands over N2K and modulates
UP and DOWN relays in response - these relays will be usually be connected
to the spudpole winch control inputs. Operating commands are subject to a
failsafe timeout: if a command is not repeated within a specified timeout
period (typically 1s), then the module will switch its output relays off.


## MODINT release history

| Version | MCU                   | Released  | Comment                   |
|:--------|:----------------------|:----------|:--------------------------|
| 1.0     | Teensy 3.2            | July 2020 | First release.            |

## N2K interface

__MODINT__ connects to the host N2K bus through a standard M12 5-pin male
connector.  The module includes a selectable bus termination resistor
allowing connection as a terminating device on the host bus.

In addition to the usual N2K network management messages the module
generates the following message types:

| PGN    | Message name                      | Freq of transmission       |
|:-------|:----------------------------------|:---------------------------|
| 128776 | Anchor Windlass Control Status    | 5s (passive), 500ms (active) |
| 128777 | Anchor Windlass Operating Status  | _ditto_                    |
| 128778 | Anchor Windlass Monitoring Status | _ditto_                    |

The module responds to the following control message types:

| PGN    | Message name                      | Timeout                    |
|:-------|:----------------------------------|:---------------------------|
| 126208 | Command Group Function            | 250ms                      |

## Hardware interfaces

__INSTANCE__.  This 9-pin DIP switch allows the installer to enter the
instance address of the spudpole by setting the left-hand eight switches to
a binary address. The right hand switch enables the built-in 120-ohm NMEA
bus termination resistor.

__SENSOR__.  Allows connection of a 5VDC inductive proximity sensor
whose output will be used to count windlass revolutions.

__DK__ &  __ST__. Allow connection of 12/24VDC signals from the spudpole
docked and stopped sensors respectively.

__UP__ & __DN__. Allow connection of control signals for the winch. The
associated relays can handle a maximum of 4A at 12VDC.

__NMEA__. Allows connection of the M12 5-pin CAN bus connector.


## Firmware

Firmware is implemented using the Anduino IDE and consists of two components:

* __N2kSpudpole__   a library class specialising the generic __Spudpole__ with support
                    for NMEA 2000. See the GitHub
                    [N2kSpudpole](https://www.github.com/preeve9534/N2kSpudpole/)
                    repository for more information.

* __modint__        an Arduino sketch providing hardware and N2K interfaces for an
                    __N2kSpudpole__ instance.

### Configuring and compiling the firmware

Every firmware build is tailored to a particular spudpole and a configuration
file ```config.h``` must be placed in the ```modint/``` directory before a
build is attempted.  The data in this file will be embedded in the firmware.
An example of a typical configuration file is shown below and the following
table descrbes each property.
```
// config.h
// Customer: REEVE
// Vessel name: BEATRICE OF HULL
// Spudpole: 1 of 1

unsigned long CONFIG_PRODUCT_CODE = 1;
char CONFIG_MANUFACTURER_CODE[] = "Ankreo";
char CONFIG_MODEL_CODE[] = "Type 32";
char CONFIG_SERIAL_CODE[] = "1001";
double CONFIG_SPOOL_DIAMETER =  0.06;
double CONFIG_LINE_DIAMETER = 0.01;
unsigned int CONFIG_SPOOL_WIDTH = 10;
unsigned int CONFIG_LINE_TURNS_WHEN_DOCKED = 60;
double CONFIG_CONTROLLER_VOLTAGE = 24.0;
double CONFIG_MOTOR_CURRENT = 0;

// End of config.h
```
|Property name|Type|Example|Comment|
|CONFIG_PRODUCT_CODE|Integer|213580|Product code reuired by NMEA when registering module on bus.|
|CONFIG_MANUFACTURER_CODE|String|"Blobby PLC"|Name of module manufacturer.|
|CONFIG_MODEL_CODE|String|String|"Type 32"|

Once a config file is in place, the build can proceed in the usual way and
the resulting firware be written to the Teensy MCU.


