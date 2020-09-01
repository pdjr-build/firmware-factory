# sketch/

The ```sketch/``` folder is a PlatformIO project which provides a build
environment for the compilation of all firmware.
The standard PlatformIO build arrangement is tweaked in the following
ways.

1. The ```sketch/src/``` folder is replaced with a symbolic link to a
   folder which contains the specific firmware source files that should
   be compiled by a build.
   
2. The ```platformio.ini``` configuration file is elaborated to execute
   ```utils/get-config``` before every build by the addition of the line:
   ```
   build_flags = ! utils/get-config src *-cfg > src/build.h
   ```

This tweaking allows firmware code to be configured by inclusion of an
automatically generated header file containing #define statements which
reflect configuration data that is distributed across a file system
hierarchy.
With careful construction of the build filesystem it is possible to
define configuration values in the contexts to which they relate and to
re-use configuration data without the need for extensive and error prone
maintenance of multiple configuration files.

## A simple example

Let's assume that we have some project that implements a module called 'FOO'
which has both hardware and firmware elements.
Hardware and firmware are both available in multiple versions and some firmware
versions are only suitable for some hardware versions.
We organise our filesystem like this.
```
pdjr/
├── MANUFACTURER_NAME
├── STOP
├── sketch/
│   ├── lib/
│   ├── src -> ../FOO/hardware/1.0/firmware-1.0/
│   ├── test/
│   ├── utils/
│   └── README.md
└── FOO/
    ├── MODULE_NAME
    ├── firmware
    │   ├── 1.0
    │   │   ├── main.cpp
    │   │   └── FIRMWARE_VERSION
    │   └── 2.0
    │       ├── main.cpp
    │       └── FIRMWARE_VERSION
    └── hardware
        ├── 1.0
        │   ├── HARWARE_VERSION
        │   ├── firmware-1.0 -> ../../firmware/1.0/
        │   └── kicad/
        └── 2.0
            ├── HARWARE_VERSION
            ├── firmware-1.0 -> ../../firmware/1.0/
            ├── firmware-2.0 -> ../../firmware/2.0/
            └── kicad/
```
Note the use of symbolic links in the hardware version folders which
capture the semantics of which firmware versions run on the hardware
version in question.

Note also that the PlatformIO project ```src/``` folder is a symbolic
link to the required firmware folder in the context of the harware
version to which our required build relates.

When we execute a PlatformIO build in ```sketch/``` the __get-config__
program will follow the ```src``` link and will then climb back up the
filesystem processing directories in the order:

1. pdjr/FOO/firmware/1.0/
2. pdjr/FOO/hardware/1.0/
3. pdjr/FOO/hardware/
4. pdjr/FOO/
5. pdjr/

Once __get-config__ reaches the ```pdjr/``` directory the presence of
the file called ```STOP``` will stop it climbing the filesystem. 

In each directory visited, configuration files identified by their
all-upper-case file names are processed into #define statements.
The filename itself becomes the name of the #define and the content
of the file becomes the #define value.
At the end of directory processing the set of generated #define
statements will be written int ```src/build.h```.

From a terminal in the ```sketch``` folder you can review the values
that will be recovered by this process as it traverses the ```src/```
hierarchy by executing the ```get-config``` script directly.
For example:
```
$> utils/get-config
#define FIRMWARE_VERSION "1.0"
#define HARDWARE_VERSION "1.0"
#define MANUFACTURER_NAME "PDJR.EU"
#define MODULE_NAME "FOO MODULE"
$>
```

# get-config

The __get-config__ script takes as its arguments a list of terminal
directory names from each of which it will begin its upward walk of the
filesystem hierarchy.
The configuration in ```platformio.ini``` will make the script process
the ```src``` folder and also any folders whose names end with "-cfg" and
it is therefore a simple matter to add additional configuration trees
which are outside of the ```src``` hierarchy by just dropping into the
```sketch/``` folder appropriately names symbolic links.
