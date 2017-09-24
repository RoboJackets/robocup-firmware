# GT RoboJackets RoboCup SSL - Firmware
[![Build Status](https://circleci.com/gh/RoboJackets/robocup-firmware.svg?&style=shield)](https://circleci.com/gh/RoboJackets/robocup-firmware)

The Georgia Tech RoboJackets team competes in the annual RoboCup Small Size League (SSL) competition.  This repository contains all of the firmware that we run on our robots. More information on how our software works can be found on our [documentation page](http://robojackets.github.io/robocup-firmware/), our [wiki](http://wiki.robojackets.org/w/RoboCup_Software) or on our [website](http://www.robojackets.org/).
Also, check out our [2014 qualification video](https://www.youtube.com/watch?v=H3F9HexPLT0) to see our robots in action!

## The Competition

The soccer game is played between two teams of six robots each on a field with overhead cameras.  The field vision computer processes images from the cameras and sends out (x,y) coordinates of the ball and the robots.  Each team has a laptop that accepts the data from the vision computer and uses it to strategize, then send commands to robots on the field.

The official [RoboCup site](http://robocupssl.cpe.ku.ac.th/) has more information on the competition.


## Project Layout

### src/

The src folder contains the firmware code for the different targets that are compiled for the robot/base. See the src [README](src/README.md) for more info.

### lib/

The lib folder contains the libraries used for the compiling targets in the src folder. This contains the source for drivers, modules, and common.


### run/

Compiled programs and some configuration files are stored here.


## Setup

Here's a quick guide to getting this RoboCup project setup on your computer.  We recommend and only provide directions for installing on Ubuntu Linux, Arch Linux, and Mac OS X, although it shouldn't be too difficult to port to other operating systems.

1) Clone the repository

```
git clone git://github.com/RoboJackets/robocup-firmware
```


2) Install the necessary software

There are a few setup scripts in the util directory for installing required packages, setting up udev rules, etc.  See `ubuntu-setup`, `arch-setup`, and `osx-setup` for more info.

```
$ cd robocup-firmware
$ ./util/<SYSTEM>-setup
```

3) Build the project for the desired target. The `control` target is the firmware for the MBED. The `fpga` target is for the FPGA to be uploaded to the MBED. The `kicker` target is for the kicker MCU to be uploaded to the MBED. The `base` target is for the base station firmware to be uploaded to the base station MBED.

```
$ make <TARGET>
```

Make targets can be uploaded automatically to the MBED by appending `-prog` to the end of the target name.

We use CMake as our build system and have a simple `makefile` setup that invokes CMake.


## Documentation

We use [Doxygen](www.doxygen.org) for documentation.  This allows us to convert specially-formatted comments within code files into a nifty website that lets us easily see how things are laid out.  Our compiled doxygen documentation can be found here:

http://robojackets.github.io/robocup-firmware/

Note: The doxygen documentation site above is updated automacally using circle-ci.  See our autoupdate-docs.sh file for more info.


## Testing

Firmware tests can be written and placed in `src/hw-test` with the name `test-<TESTNAME>.cpp` then compiled with `make robot-test-<TESTNAME>`

Generic firmware tests can be run with `make test-firmware`. (Still not completely sure what this does)


## License

This project is licensed under the Apache License v2.0.  See the [LICENSE](LICENSE) file for more information.

