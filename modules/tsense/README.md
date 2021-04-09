# TSENSE - NMEA 2000 temperature sensor module

__TSENSE__ is an NMEA 2000 module which allows the connection of up to
eight LM335Z temperature sensors.
The module transmits temperature readings from the connected sensors
over NMEA 2000 using [PGN 130316 Temperature, Extended Range](
https://www.nmea.org/Assets/nmea%202000%20pgn%20130316%20corrigenda%20nmd%20version%202.100%20feb%202015.pdf).

__TSENSE__ connects to a host NMEA bus by a standard M12 5-pin circular
connector and is powered directly from the NMEA bus.
The module has an NMEA LEN of 1.

Status and diagnostic LEDs confirm NMEA connection and module operating
status.
The module is configured by a PCB mounted DIP switch which allows entry
of NMEA instance address, temperature source and set temperature for
each connected sensor.

Multiple __TSENSE__ modules can be installed on a single NMEA bus.

## About the module

![Fig 1: module schematic](tsense.png.svg)

Figure 1 illustrates the appearance of the module with the cover in
place and with the cover removed.

The top cover includes the NMEA bus connector (1) and a status
LED (2). The cover is penetrated by two cable glands (3) which allow
passage of sensor connection cables.

The top cover is released by pinching at (A) after which it can be
lifted away from the the base to expose the printed circuit board (PCB).
The PCB has sensor wire terminals (4), a programme switch (5), a
configuration DIL switch (6) and three configuration state LEDs (7).
A jumper (8) allows connection of the NMEA cable shield to the module
ground.

### (1) NMEA bus connector

The module uses an M12 circular female 5-pin industrial connector for
NMEA connection.
Any standard compliant N2K drop cable will allow the module to be connected
to a host NMEA bus through a T-connector.

### (2) Status LED

A PWR status LED is modulated to indicate operating status.

| LED    | Illumination state                | Meaning |
|:------:|:----------------------------------|:--------|
| PWR/TX | *n* rapid flashes on startup      | The module has just been connected to power and is initialising *n* temperature sensors.|
|        | Steady                            | The module has power.|
|        | Flashing                          | The module is transmitting NMEA data from a configured sensor. |

### (3) Cable glands

Two 6mm cable glands allow passage of switch and indicator connection
cables.

### (4) Sensor wire terminals

Two 8-pin terminal blocks allow the connection of up to eight two-wire
LM335Z temperature sensors, numbered 0 through 7. Each sensor requires
a two-wire connection to its respective positive (P) and ground (G)
pins; the LM335Z calibration pin is not used.

| Terminal | Function                        |
|:--------:|:--------------------------------|
| [0..7]-P | Positive connection to sensor   |
| [0..7]-G | Ground connection to sensor     |

DO NOT connect resistive or milliamp temperature sensors to these
terminal inputs.

### (5) Programme switch

The PRG programme switch is a momentary action push button which is
used together with DIL switch (6) to configure each connected sensor
channel's NMEA properties.

### (6) DIL switch

The DIL switch allows the selection of a sensor channel which is to be
programmed and the entry of configuration data.
Eight slide switches are each labelled with a value 0 through 7 which
can be used to select a temperature sensor channel for programming and
allow the entry of binary encoded configuration data. 

### (7) Programme state LEDs

Programming a sensor channel requires the consecutive entry of two or
three numeric values and these LEDs provide information on the state
of progress through the programming activity.

### (8) SCR jumper

With the jumper in place the NMEA cable shield is connected to the module
ground plane.

## Installing the module

1. Position the module close to the switches and indicators which you intend
   to use for operation and feedback, ensuring that you are able to remove
   the top cover for access to the PCB switches and that you have space to
   route the various connecting cables.
   Fix the module to its supporting surface with appropriate fasteners.

2. Ensure that there is a T-connector or multi-drop connector available on
   the NMEA bus you have chosen to host the module and connect an NMEA drop
   cable to the module and route it to the bus connector.
   If your host NMEA bus is poered, do not connect the drop cable to the bus
   at this stage: the module must remain unpowered until installation is
   complete.
   
3. Carefully remove the module cover.
   Inside the module, the NMEA connector is wired to the PCB with a 150mm
   long cable which should allow you to conveniantly position to one side
   so that you can access the module connector blocks.
   Do not place excessive strain on the internal NMEA connector cable.
   
4. Wire the switches and indicators that constitute your chosen control
   interface.
   The minimum functional requirement for any channel is that U0 and D0
   are connected to an operating switch of some form.
   
   Fig 2 gives an illustrative schematic for a simple wiring arrangement
   that incorporates a SPDT switch and three indicators in support of
   a single windlass channel.
   Note the use of jumpers on the indicator outputs to allow the use of
   the PWR GND connection for outputs 0UP and 0DN.
  
5. The power supply to the module should be fused at a level that supports
   your chosen indicators (the switch input current requirement is
   negligible at around 40mA).
   .
![Fig 2: Wiring example](wiring.png)
   
## Configuring the module

To operate correctly the module must be configured with the NMEA instance
of the windlasses which are being controlled.
If only one windlass is to be controlled then a special instance value can
be used to disable the unused channel.
The module is supplied with channe W0 unconfigured and channel W1 disabled.

To configure a windlass control channel.

1. Inspect the windlass which is to be controlled and identify the instance
   number that has been assigned to its NMEA interface.
2. Connect the module to the NMEA bus and confirm that it has power.
3. If you have connected switches to the SWITCH terminal block, then
   ensure that the switches are in the OFF position.
4. Enter the instance number of the windlass you wish to control (i.e the
   value identified at (1)) using the DIP switch on the module PCB.
   The module allows instance numbers in the range 0 through 126 to be
   entered.
   An instance number of 127 can be used to disable a control channel.
5. Press the PRGW0 button on the module PCB to store the windlass instance
   number for channel W0 or PRGW1 to store the instance number for channel
   W1.

If the NMEA interface on the remote spudpole is powered up, then after
a few seconds the module status LED for the selected control channel should
turn off and the status outputs for windlass should become active to indicate
the condition of the windlass.
After this, the switch inputs for the channel should command the remote
windlass.

You can repeat this procedure at any time to update a configuration or
programme a second windlass.

### Configuration issues

Configuration problems are indicated by the status LEDs on the module top
cover.
After programming an instance number to a control channel, the corresponding
channel status LED should turn off.

### Status LED flashes twice and repeats.
The module channel has not been programmed - i.e. a windlass instance number
has not saved to module memory.

1. Re-programme the instance number.
   Make sure that you press and hold the relevant programme switch for one second.
   
### Status LED flashes once and repeats for more than ten seconds.
No messages have been received from the windlass identified by the programmed
instance number.

1. Make sure the remote windlass interface module is powered.
2. If you have more than one NMEA bus, make sure that both the windlass and
   the control module are connected to the same bus or that the busses are
   bridged.
3. Check/replace the drop cables used at both the windlass and the control
   module.
4. Check the instance number of the windlass and programme it into the module
   again.
5. Use an NMEA monitor programme to confirm that the remote windlass is
   transmitting PGN 128777 Windlass Operating Status messages.






## NMEA 2000 messages

In addition to the usual N2K network management messages the module
accepts the following message types and processes them to drive its
status outputs.

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 128776 | Anchor Windlass Control Status    | Drives status outputs |
| 128777 | Anchor Windlass Operating Status  | Drives status outputs |

The module issues the following control message types in response to
signals on the switch inputs.

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 126208 | Command Group Function            | 250ms transmition rate|


