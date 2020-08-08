# /ankreo/sketch

This folder is the build environment for all firmware and is configured as a
standard PlatformIO project with the following directory structure.
```
├── lib/                      <- application specific libraries
├── src/                      <- application source files
├── test/                     <- application test files
├── utils/                    <- local build system utilities
└── README.md                 <- this file!
```
Before attempting a build you must at least create some symbolic links which
will tailor the upcoming build to your specific requirements. You may also
need to modify some PlatformIO settings to suit your needs (for example,
to select a specific target board).

1. Create a sympolic link called ```src/main.cpp``` to the Arduino sketch you
   want to compile.  Typically these will be located in the device's firmware
   folder hierarchy.  For example:
   ```
   $ ln -s ../spudpole/modint/firmware/1.0/sketch.ino src/main.cpp
   ```  

2. Create a symbolic link called ```hardware``` to the module hardware version
   you are targetting.  For example:
   ```
   $ ln -s ../spudpole/modint/hardware/1.0/ hardware
   ```

3. Create a symbolic link called ```firmware``` to the module firmware version
   you are targetting.  For example:
   ```
   $ ln -s ../spudpole/modint/firmware/1.0/ firmware
   ```

4. Create a sympolic link called ```product``` to the product model you are
   targetting. For example:
   ```
   $ ln -s ../spudpole/spudpole-types/ES1001-3000-12/ product
   ```
The final configuration step is to ensure that PlatformIO updates main.cpp
with values harvested from variable definitions in the filesystem hierachies
identified above.  This requires the following entry in the PlatformIO
configuration file.
```
build_xxx = "! utils/get-values hardware firmware product | utils/interpolate src/main.cpp"
```
With this configuration in place, all that is necessary to attempt a build
(i.e. to compile ```main.cpp```) is to invoke the PlatformIO build process.
I tend to use VSCode with the PlatformIO plugin, so this all do-able from
within VSCode.

## How the local build system works

The local build system works by harvesting variable definitions from a
directory hierarchy.  You specify leaves in the hierarchy when you make
the symbolic links described above and the build system climbs back up
the branches to the ```ankreo/``` root folder, gathering variable
definitions as it goes.

With a hierarchy like ```ankreo/spudpole/modint/hardware/1.0/``` variable
definitions can be located in the folders with children to which they apply
and do not have to be duplicated or embedded in hard-to-maintain fixed
configuration files. The build system also manages automatic incrementing
of serial numbers.

From the ```sketch``` folder you can see the results of this process by
typing:
```
utils/get-config
```
which will list the configuration variables in this and the parent directory
```ankreo```.  You'll get a bigger list if you type:
```
utils/get-config hardware
```

The Makefile uses ```utils/get-config``` to generate a list of all variables
pertinent to your build by running:
```
utils/get-config firmware hardware product
```
and the using the ```utils/interpolate file.cpp``` command to process these values
into ```file.cpp```.
