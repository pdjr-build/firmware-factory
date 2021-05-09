# firmware-factory - build environment for Arduino style projects.

firmware-factory provides a PlatformIO project folder ```sketch/```
(which can be configured to meet local needs) and a small collection
of tools which elaborate PlatformIO features.

The actual source code that the factory will build is tied into
the build environment using a symbolic link ```sketch/src``` which
must be set before attempting a PlatformIO build.
Simply switch to the ```sketch/``` folder and make a symbolic link
to the folder which contains your firmware source code.
For example:
```
firmware-factory> cd sketch
sketch> mkdir ~/mymodule src
```

The build environment includes an extension to the PlatformIO build
process that attempts to construct a file ```build.h``` from text
*definition files* harvested by climbing directories in file system
from ```src/``` until the root folder of firmware-factory.

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
