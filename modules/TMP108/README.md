# TMP108 - NMEA 2000 temperature sensor module

__TMP108__ is an NMEA 2000 module which allows the connection of up to
eight LM335Z temperature sensors.
The module transmits temperature readings from the connected sensors
over NMEA 2000 using [PGN 130316 Temperature, Extended Range](
https://www.nmea.org/Assets/nmea%202000%20pgn%20130316%20corrigenda%20nmd%20version%202.100%20feb%202015.pdf).

__TMP108__ connects to a host NMEA bus by a standard M12 5-pin circular
connector and is powered directly from the NMEA bus.
The module has an NMEA LEN of 1.

Status and diagnostic LEDs confirm NMEA connection and module operating
status.
The module is configured by a PCB mounted DIP switch which allows entry
of NMEA instance address, temperature source and set temperature for
each connected sensor.

Multiple __TMP108__ modules can be installed on a single NMEA bus.

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

The module uses an M12, circular, female, 5-pin industrial connector
for NMEA connection.
Any N2K standard-compliant drop cable will allow the module to be
connected to a host NMEA bus through a T-connector.

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
programmed and the entry of binary-encoded sensor configuration data.
The switch consists of eight small slide switches labelled one through
eight. 

### (7) Programme state LEDs

There are three LEDs labelled INST, SRCE and SETP which are used to
indicate what data has been entered and what is required whilst a sensor
channel is being configured.

During normal module operation these LEDs will all be switched off.

### (8) SCR switch

With the switch closed the NMEA cable shield is connected to the module
ground plane.

## Temperature sensors

The TMP108 module supports a maximum of eight LM335Z or equivalent
temperature sensor IC.
Other types of temperature sensor cannot be used and connecting them
to a TMP108 module will almost certainly damage the module beyond
repair.

LM335Z-based temperature sensors are commercially available (for
example Victron ) or you can easily make your own sensor.

The LM335Z sensor IC is packaged in a TO-92 housing and has the
following pin layout.

Pin 1 - calibrate (not used by TMP108)
Pin 2 - ground (G)
Pin 3 - supply (P)

### Making a bolt-on temperature sensor

You will require an LM335Z IC, a length of two-core cable with 0.5mm2
conductors, a 10mm2 ring terminal with a hole size that suits your
mounting needs and a minimum 10mm2 cable capacity and some silicone
sealer or potting compound.

1. Remove the calibrate pin from the LM335Z by cutting or breaking it
   off as close to the IC body as possible.

2. Solder an appropriate length of two-core cable to IC pins G and P.
   Make sure you can identify which pin connects to which cable core!
   Insulate the exposed connection is some way, perhaps using small
   diameter heat-shrink sleeving around each connection.

3. Fill the cable-entry port of the ring terminal with silicone-sealant
   and coat the LM335Z with sealant as well.
   Try to avoid air bubbles.

4. Fully immerse the LM335Z in the ring-terminal cable-entry port so
   that the electrical connections are completely embedded in the
   sealant and making sure that they do not touch the ring-terminal
   body.

5. Allow the sealant to fully cure.

Ordinary silicone-sealant and potting compounds are not wonderful
conductors of heat so it is good to keep the volume of these low.
Thermally conductive silicone-sealant and potting compounds perform
better, but are expensive.

## Installing and configuring the module

### Removing and replacing the module cover

The following procedures require you to remove the module cover in
order to gain access to PCB mounted terminals and switches.

To remove the module cover pinch firmly at the centre of the top and
botton edges and pull away from the base.
The NMEA connector is wired internally to the PCB with a 150mm long
cable which you should be careful not to strain excessively.
The length of cable  should allow you to conveniantly position the
cover to one side of the module base so that the PCB can be easily
accessed.
   
To replace the module cover arrange the NMEA connector cable so that
it will not be trapped between base and cover and push the cover onto
the base until it clicks in place.   

### Installing the module

1. Position the module so that you are able to connect the associated
   temperature sensors and NMEA drop-cable without excessive cable
   runs.
   Ensure that in your chosen position you are able to remove the
   module's top cover and can easily access the PCB terminals and
   switches.

2. Fix the module in your chosen location using appropriate fasteners.

### Connecting the module to the host NMEA bus

You will require a T-connector or an unused tap on a multi-drop
connector to provide a drop-cable connection point on host NMEA bus.

1. Connect an NMEA drop-cable between between the host NMEA bus and the
   module's NMEA connector.
   The module's power LED will illuminate to indicate that the module
   has power.

### Connecting temperature sensors

1. Disconnect the module NMEA cable or otherwise power down the host
   NMEA bus.

2. Remove the module cover.
   
3. Thread a temperature sensor cable through one of the entry glands
   in the module cover and connect to an unused terminal pair on the
   PCB.
   Use the G terminal of each terminal pair for the sensor ground and
   the corresponding P terminal for the sensor power supply.

   Repeat this step for each temperature sensor you wish to connect.

4. Reconnect the module NMEA cable or power-up the host NMEA bus.
   
5. Make a note of the purpose of each newly connected sensor in the
   "Comment" column of the installation table on page NNN.

## Configuring a sensor connection

You can configure a sensor connection before or after the connection
of a temperature sensor.

For the module to operate correctly each sensor connection must be
assigned an NMEA instance address, a code which describes the type of
data source associated with the temperature reading and an alarm
temperature or set point.

### Preparing for configuration

For each sensor recorded in the installation table on page NNN, insert
a value into the "Instance", "Source" and "Set-point" columns that
reflects your installation requirements and satisfies the following
constraints.

INSTANCE. The value you choose must be in the range 0 through 255 and
must be a unique identifier for each temperature sensor across the
vessel's entire NMEA installation.
The value 255 is used to indicate that a sensor connection is disabled.

SOURCE. The value you choose must be in the range 0 through 255 and
should be selected from the table included as "Annex 1: NMEA
temperature source codes".
Some standard values are defined in NMEA and will either be appropriate
or should be avoided.

SET-POINT. The value you choose must be in the range 0 through 255 and
should specify a set-point or alarm temperature in degrees Celsius.
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

We begin programming by selecting the sensor we wish to configure on
the DIL switch.
The left-most switch corresponds to sensor 0, the right-most switch
to sensor 7.
We are programming sensor 0, so we set the DIL switch to [ON-OFF-OFF-OFF-OFF-OFF-OFF-OFF]
and then press PRG.

If our entry is accepted then the INST LED will flash to indicate that
the module is awaiting entry of a sensor instance value.
At this point, we have 20 seconds in which to set up the requested
instance value on the DIL switch and press PRG to confirm the entry.

We are setting an instance value of 0, so we set the DIL switch to
[OFF-OFF-OFF-OFF-OFF-OFF-OFF-OFF] and press PRG.

If our entry is accepted, then the INST LED will become steadily
illuminated and the SRCE LED will begin to flash, requesting entry of a
source value within a new 20s timeout period.

We are setting a source value of 11, so we set the DIL switch to
[OFF-OFF-OFF-OFF-ON-OFF-ON-ON] and press PRG.

If our entry is accepted then the SRCE LED will become steadily
illuminated and the SETP LED will begin to flash, requesting entry of a
set-point / alarm value within a new 20s timeout period.

We are setting a set-point value of 83, so we set the DIL switch to
[OFF-ON-OFF-ON-OFF-ON-OFF-ON] and press PRG.

If our entry is accepted then the INST, SRCE and SETP LEDs will flash
three times to indicate that all enries have been accepted and the
configuration has been saved to the module's EEPROM.

The protocol described above can be repeated to programme another
sensor channel or to amend an incorrect or inappropriate existing
entry.

If an error is made in the programming sequence or the 20s timeout
expires before an entry is made, then the all LEDS will extinquish and
the programming sequence must be restarted with the selection of the
sensor to be programmed.

## NMEA 2000 output

As soon as a temperature sensor has been programmed the module will
commence transmitting the sensor's temperature reading onto the host
NMEA bus.

The module issues the following control message types in response to
signals on the switch inputs.

| PGN    | Message name                      | Comment               |
|:-------|:----------------------------------|:----------------------|
| 126208 | Command Group Function            | 250ms transmition rate|


