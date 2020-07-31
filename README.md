# ankreo
Build system supporting development of NMEA 2000 modules for Ankreo products.

This project implements a directory hierarchy supporting the development of
NMEA 2000 interfaces to Ankreo products.

The project root directory has the structure:
```
README.md
LICENSE
DEVICE_MANUFACTURER_NUMBER
DEVICE_INDUSTRY_GROUP_NUMBER
sketch/
spudpole/
```
README.md (this file) and LICENSE are just standard repository content.

DEVICE\_MANUFACTURER\_NUMBER and DEVICE\_INDUSTRY\_GROUP\_NUMBER are part of
the build system value definition mechanism which is built around a collection
of similar distributed files, each of which defines a variable name (by the
file name) and its value by the file content.  The build system includes a
mechanism for gathering such files and interpolating them into an Arduino
source code.

sketch/ is the Arduino sketch folder in which all builds are executed. Review
the README.md file in this folder for details of how to use the build system.

The remaining folders in this directory define particular Ankreo product lines
(in this example, the only product line is spudpole/0.
