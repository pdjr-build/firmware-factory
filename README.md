# ankreo

Build system supporting development of NMEA 2000 modules for Ankreo products.

The build system consists of:

1. a PlatformIO project ```build/``` which is used as the compile environment
   for all firmware development;
2. a hierarchical directory structure used to organise assets, and;
3. a collection of tools which configure the build environment by
   processing assets.

See the file ```build/README.md``` for a description of the PlatformIO
project's integration with the build system.

The directory structure used to organise project assets consists of an
arbitrary number of product-line directories located in the ```ankreo/```
root. The current system supports just spudpole products and the only
root folder is ```spudpole/``` which is organised in the following way.

```
spudpole/
├── modctl/                                     <- A directory for each N2K device.             
├── moddsp/                                 
├── modint/                                
│   ├── DEVICE_CLASS_NUMBER              
│   ├── DEVICE_FUNCTION_NUMBER
│   ├── firmware/                               <- folder for device firmware
│   │   └── 1.0/                                <- folder for a firmware version
│   │       ├── FIRMWARE_VERSION
│   │       └── main.cpp                        <- the Arduino sketch implementing this firmware version
│   ├── hardware/                               <- folder for device hardware
│   │   └── 1.0/                                <- folder for hardware version
│   │       ├── kicad/                          <- folder for hardware design
│   │       ├── pcb.pdf                         <- for use in README.md
│   │       ├── PRODUCT_CERTIFICATION_LEVEL_NUMBER
│   │       ├── PRODUCT_LEN_NUMBER
│   │       ├── PRODUCT_N2K_VERSION_NUMBER
│   │       ├── PRODUCT_VERSION
│   │       ├── README.md                       <- description of this hardware version
│   │       └── schematic.pdf                   <- for use in README.md
│   ├── PRODUCT_CODE_NUMBER
│   ├── PRODUCT_SERIAL_CODE
│   └── PRODUCT_TYPE
├── README.md                                   <- description of this device
└── spudpole-types/                             <- container for configuring spudpole types
    ├── ES1001-3000-12/                         <- folder for a spudpole type
    │   ├── SPUDPOLE_CONTROLLER_VOLTAGE_NUMBER
    │   ├── SPUDPOLE_LINE_DIAMETER_NUMBER
    │   ├── SPUDPOLE_LINE_TURNS_WHEN_DOCKED_NUMBER
    │   ├── SPUDPOLE_MOTOR_CURRENT_NUMBER
    │   ├── SPUDPOLE_SPOOL_DIAMETER_NUMBER
    │   └── SPUDPOLE_SPOOL_WIDTH_NUMBER
    ├── ES1001-3000-24/                         <- folder for another spudpole type
    │   ├── SPUDPOLE_CONTROLLER_VOLTAGE_NUMBER
    │   ├── SPUDPOLE_LINE_DIAMETER_NUMBER
    │   ├── SPUDPOLE_LINE_TURNS_WHEN_DOCKED_NUMBER
    │   ├── SPUDPOLE_MOTOR_CURRENT_NUMBER
    │   ├── SPUDPOLE_SPOOL_DIAMETER_NUMBER
    │   └── SPUDPOLE_SPOOL_WIDTH_NUMBER
    └── README.md
```
In addition to the obvious assets you will notice that the file system
contains a number of _DEFINITION\_FILES_ with capitalised, underscore-delimited
names. The names correspond to configuration variable and constant names used
in program code and their placement in the filesystem indicates the children
to which these configuration values apply.

Definition files are assumed to contain a string value unless their name
ends in '_NUMBER' in which case they are taken to contain a numeric value
(the '_NUMBER' suffix is not considered to be part of the value name). For
example DEVICE\_MANUFACTURER\_NUMBER file will be assumed to contain a
numeric value for a variable called 'DEVICE_MANUFACTURER'.

The content of a definition file should be the value to be used for the
named variable: quotes are not required on strings and numbers can be
expressed in whatever way the host application demands.
