# ankreo
Build system supporting development of NMEA 2000 modules for Ankreo products.

This project implements a directory hierarchy supporting the development of
NMEA 2000 interface modules for Ankreo products.

The project root directory has the structure:
```
README.md
LICENSE
DEVICE_MANUFACTURER_NUMBER
DEVICE_INDUSTRY_GROUP_NUMBER
sketch/
spudpole/
```
README.md (this file) and LICENSE are standard repository content.

DEVICE\_MANUFACTURER\_NUMBER and DEVICE\_INDUSTRY\_GROUP\_NUMBER are part of
the build system code configuration mechanism which allows redefinition of
values defined in source code from a collection of distributed files each of
which defines a variable name (by its file name) and a value by the file
content.

sketch/ is the Arduino sketch folder in which all builds are executed. Consult
the README.md file in the sketch/ folder for a detailed description of the
build system and how to use it.

The remaining folders in this directory define particular Ankreo product lines
(in this example, the only product line is spudpole/).  The organisation of
these folders is arbitrary, but the default scheme looks like this.
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
│   │       └── sketch.ino                      <- the Arduino sketch implementing the firmware version
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

