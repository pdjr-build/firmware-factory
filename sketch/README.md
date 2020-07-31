# /ankreo/sketch

This folder is the build environment for all firmware and has the following
structure:
```
├── libraries/                <- the Arduino expects project libraries here
├── Makefile                  <- build system Makefile
├── README.md                 <- this file!
└── utils/                    <- scripts making implementing the build system
```
You will notice there is no Arduino sketch here.  Before attempting a build you
must create some symbolic links which will tailor the upcoming build to your
specific requirements.

1. Create a sympolic link called ```sketch.ino``` to the Arduino sketch you want
   to compile.  Typically these will be located in the device's firmware folder
   hierarchy.  For example:
   ```
   $ ln -s ../spudpole/modint/firmware/1.0/sketch.ino .
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

## How the build system works

The build system works by harvesting variable definitions from a directory
hierarchy.  You specify leaves in the hierarchy when you make the symbolic
links described above and the build system climbs back up the branches to
the ```ankreo/``` root folder, gathering variable definitions as it goes.

With a hierarchy like ```ankreo/spudpole/modint/hardware/1.0/``` variable
definitions can be located in the folders with children to which they apply
and do not have to be duplicated or embedded in hard-to-maintain fixed
configuration files.

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
and the using the ```utils/interpolate``` command to process these values
into ```sketch.ino```.

Finally, the Makefile runs the Arduino compiler against the refreshed
sketch.

The build system runs from the command line.  You must run a new build from the
command line at least once before you start using the Arduino IDE or some other
programming environment.
