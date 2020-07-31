#/ankreo/sketch

This folder is the build environment for all firmware and has the following
structure:
```
├── libraries/
├── Makefile
├── README.md
└── utils/
```
libraries/ is the usual Arduino library folder.

Makefile is a Makefile which implements the build system and starts compilation
of an Arduino sketch.

README.md is this file.

utils/ is a folder containing scripts used by the build system.

You will notice there is no Arduino sketch here.  Before attempting a build you
must create some symbolic links which will tailor the build to your specific
requirements.

1. Create a sympolic link to the Arduino sketch you want to compile.  Typically
   these will be located in the device's firmware folder hierarchy.  For example:
   ```
   $ ln -s ../spudpole/modint/firmware/1.0/sketch.ino .
   ```  
   Restrictions in the Arduino build process mean that these files had better
   be called ```sketch.ino``` or at least aliased by the link to ```sketch.ino```.

2. Create a symbolic link ```hardware``` to the module hardware version you
   are targetting.  For example:
   ```
   $ ln -s ../spudpole/modint/hardware/1.0 hardware
   ```

3. Create a symbolic link ```firmware``` to the module firmware version you
   are targetting.  For example:
   ```
   $ ln -s ../spudpole/modint/firmware/1.0 firmware
   ```

4. Create a sympolic link to the product model you are targetting. For example:
   ```
   $ ln -s ../spudpole/spudpole-types/ES1001-3000-12 product
   ```

Your build directory will now look something like this:
```
├── firmware -> ../spudpole/modint/firmware/1.0/
├── hardware -> ../spudpole/modint/hardware/1.0/
├── libraries/
├── Makefile
├── product -> ../spudpole/spudpole-types/ES1001-3000-24/
├── README.md
├── sketch.ino -> ../spudpole/modint/firmware/1.0/sketch.ino
└── utils
```
With these links in place, all that is necessary to attempt a build (i.e. to
compile ```sketch.ino```) is to type:
```
$ make
```
