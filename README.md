# firmware-factory - build environment for PlatformIO projects.

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
*definition files* harvested by climbing directories in the file
system from some starting point until either the root of the filesystem
is reached or a folder containing the file ```STOP``` is encountered.

A *definition file* is simply a text file with a name which will be
interpreted as the name of a #define token and with content which will
be interpreted as the value of the #define token.
A definition file name must be capitalised and include at least one
underscore character.

Definition files are assumed to contain a string value unless their name
ends in '_NUMBER' in which case they are taken to contain a numeric value
(the '_NUMBER' suffix is not considered to be part of the value name). For
example DEVICE\_MANUFACTURER\_NUMBER file will be assumed to contain a
numeric value for a #define token called 'DEVICE_MANUFACTURER'.

The content of a definition file should be the value to be used for the
named variable: quotes are not required on strings and numbers can be
expressed in whatever way the host application demands.

The ```src/``` folder is always a starting point and other folders
with names of the form "\*-cfg" will also be used and start points.
The root directory of "firmware-factory" contains a ```STOP``` file.

