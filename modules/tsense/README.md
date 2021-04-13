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

### (8) SCR switch

With the switch closed the NMEA cable shield is connected to the module
ground plane.

## Installing the module

1. Position the module so that you are able to connect the associated
   temperature sensors and NMEA drop-cable without excessive cable runs
   and ensuring that you are able to remove the top cover for access to
   the PCB switches.

   Fix the module to its supporting surface with appropriate fasteners.

2. Carefully remove the module cover and position it so that you have
   easy access to the temperature sensor connection terminals.

   The NMEA connector is wired to the PCB with a 150mm long cable which
   should allow you to conveniantly position the cover to one side:
   consider temporarily taping the cover out of the way.
   Do not place excessive strain on the internal NMEA connector cable.
   
3. Connect each temperature sensors that you intend using to an unused
   terminal pair on the PCB.
   Use the G terminal of a pair for the sensor ground and the P
   terminal for the sensor power supply.
   Make a note in the "Comment" column of the installation table on
   page N of the purpose of each connected sensor.

4. Connect an NMEA drop cable between between the host NMEA bus and the
   module's NMEA connector.
   The module's power LED will illuminate to indicate that the module
   has power.

5. If you intend configuring the installed temperature sensors at this
   stage, then skip to the "Module Configuration" section below before
   continuing to step (6).

6. Replace the module cover taking care not to trap the NMEA connector
   wires.

## Module configuratiom

For the module to operate correctly each connected temperature sensor
must be configured with an NMEA instance address, a code which
describes the type of data source associated with the temperature
reading and an alarm temperature or set point.

For each sensor recorded in the installation table you prepared earlier
insert a value into the "Instance", "Source" and "Set-point" columns
that reflects your installation requirements and satisfies the
following constraints.

The value you choose for "Instance" must be in the range 0 through 255
and must be a unique identifier for each temperature sensor across the
vessel's entire NMEA installation.
The value 255 can be used to indicate that a temperature sensor is
disabled.

The value you choose for "Source" must be in the range 0 through 255
and should be selected from the table included as "Annex 1: NMEA
temperature source codes".
Some standard values are defined in NMEA and will either be appropriate
or should be avoided.

The value you choose for "Set-point" must be in the range 0 through 255
and specifies a set-point or alarm temperature in degrees Celsius.
Value below 100 are considered to be below zero Celsius, values above
100 are considered to be above zero Celsius and the value 100 specifies
0 Celsius.
Thus, the value 53 says "-53 Celsius", 147 says "+47 Celsius".

### Programming the module

The values you have defined above are programmed into the module by
setting up the required value as a binary-encoded number on the module
DIL switch and then pressing PRG to commit the DIL switch value to
the module configuration.

The programming sequence is: (1) select sensor to be programmed; (2)
enter instance; (3) enter state; (4) enter set-point.

Let us imagine we have the following temperature sensor settings:

Sensor id: 0
Instance:  0
Source:    11 (Gearbox temperature)
Set point: 85C

Begin programming by selecting the sensor you wish to configure on the
DIL switch. The left-most switch corresponds to sensor 0, the right-most
switch to sensor 7.
You must only select one sensor and then press and release PRG.

*In the case of our example we set [ON-OFF-OFF-OFF-OFF-OFF-OFF-OFF]*.

If your entry is accepted then the INST LED will flash to indicate that
the module is awaiting entry of a sensor instance value.
At this point, you have 20s in which to set up the DIL switch and press
PRG again to accept the instance entry.

*In the case of our example we set [OFF-OFF-OFF-OFF-OFF-OFF-OFF-OFF]*.

If your entry is accepted, then the INST LED will become steadily illuminated
and the SRCE LED will begin to flash, requesting entry of a source value
within a new 20s timeout period.

*In the case of our example we set [OFF-OFF-OFF-OFF-ON-OFF-ON-ON]*.

If your entry is accepted then the SRCE LED will become steadily illuminated
and the SETP LED will begin to flash, requesting entry of a set-point / alarm
value within a new 20s timeout period.

*In the case of our example we set [OFF-ON-OFF-ON-OFF-ON-OFF-ON].




The configuration is saved to EEPROM only after  
sensor at a time and the process begins by selection of the sensor
which is to be programmed.
Programming proceeds by using the DIL switch to specify a value and
the PRG button to commit the DIL switch value:

C

To configure a temperature sensor channel.

1. Select the sensor to be configured
  1.1 Set a single switch on the DIL switch that corresponds to the sensor
      you wish to configure.
  1.2 Briefly press and then release the PRG button. If your selection at
      (1.1) is valid, the INST LED will begin to flash, indicating that the
      module is waiting for you to specify an NMEA instance address for your
      selected temperature sensor.
2. Assign the selected sensor an NMEA instance address
  2.1 Using the DIL switch, setup a binary number in the range 0
      through 255 representing your chosen address.
      Bear in mind that each temperature sensor must have a unique
      instance address.
      The address 255 (all switches in the ON position) is used to
      indicate that the selected sensor is disabled.
  2.2 Briefly press and then release the PRG button.
      The INST LED will stop flashing and move to steady ON
      indicating that the instance address has been accepted.
      The SRCE LED wil begin to flash indicating that that the module
      is waiting for you to specify an NMEA temperature source code.
3. Assign the selected sensor an NMEA source code
  3.1 Using the DIL switch, setup a binary number in the range 0
      through 255 representing the type of temperature source being
      reported by your selected sensor (consult Table 3.1 for a list
      of source codes defined by the NMEA standard).
  3.2 Briefly press and then release the PRG button.
      The SRCE LED will stop flashing and move to steady ON
      indicating that the source code has been accepted.
      The SETP LED wil begin to flash indicating that that the module
      is waiting for you to specify a set point (alarm) temperature
      for your selected sensor.
4. Assign the selected sensor a set-point/alarm temperature.
  4.1 Using the DIL switch setup a binary number that represents
      your desired temperature in degrees centigrade.
  4.2 Briefly press and then release the PRG button.
      The SETP LED will stop flashing and move to steady ON
      indicating that the set point temperature has been accepted.
      After a moment or two, all three programming LEDs will flash
      three times, confirming that the entered settings have been
      saved to EEPROM.
      

theConnect the module to the NMEA bus and confirm that it has power.
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


