
# GT RoboJackets RoboCup SSL - Firmware
[![Build Status](https://circleci.com/gh/RoboJackets/robocup-firmware.svg?&style=shield)](https://circleci.com/gh/RoboJackets/robocup-firmware)

The Georgia Tech RoboJackets team competes in the annual RoboCup Small Size League (SSL) competition.  This repository contains all of the firmware that we run on our robots. More information on how our software works can be found on our [documentation page](http://robojackets.github.io/robocup-firmware/), our [wiki](http://wiki.robojackets.org/w/RoboCup_Software) or on our [website](http://www.robojackets.org/).
Also, check out our [2014 qualification video](https://www.youtube.com/watch?v=H3F9HexPLT0) to see our robots in action!

## The Competition

The soccer game is played between two teams of six robots each on a field with overhead cameras.  The field vision computer processes images from the cameras and sends out (x,y) coordinates of the ball and the robots.  Each team has a laptop that accepts the data from the vision computer and uses it to strategize, then send commands to robots on the field.

The official [RoboCup site](http://robocupssl.cpe.ku.ac.th/) has more information on the competition.

# Getting Started
If you are a new member of RoboCup Electrical or Software and are interested in getting involved with firmware it is highly recommend you start with the [getting started](docs/GettingStarted.md) pages.

## Project Layout
For a high level overview of architecture of robocup-firmware see [here](docs/Firmware.md)

### robot/
The robot folder contains the firmware code for the different targets that are compiled for the control board, FPGA, IMU, and radio.

### robot/control
Contains the firmware code for the main program run on the mtrain. This controls the overall function of the robot and the interface with various peripheral devices, such as IMU, FPGA, and the radio

Modules are fully independent sections of code that take a well defined input (if applicable), do some action, then produce a well defined output (if applicable). For example, the kicker module takes kicker commands (input), directly interfaces with the kicker and does the communication and unit translation (action), and produces breakbeam and voltage to the rest of the firmware (output).

Modules are a class following the interface found in generic_module. The module specifies how often an action should run. At that specific frequency, the entry function will be called to do the action part of the module. The inputs and outputs are given through shared structures passed in the constructor.

The drivers themselves are defined in robot/lib .

### robot/build/
Compiled binaries output from the build system for the mtrain connected to the control board get stored here in the bin/ directory as well as other build results.

### kicker/
The kicker folder contains the firmware code for the different targets that are compiled for the kicker board.
Details on the architecture of the kicker firmware code can be found [here](docs/Kicker.md)

### kicker/build/
Compiled binaries output from the build system for the ATtiny on the kicker board get stored here in the bin/ directory as well as other build results.

### fpga/
Contains the Verilog code to be programmed to the FPGA for motor control.
Details on the architecture of the FPGA firmware code can be found [here](docs/FPGA.md)


## Setup
Below is a quick guide to getting this RoboCup project setup to build on your computer. If you are a robocup member planning on developing both mtrain and robocup firmware it is highly recommended that you  follow the Firmware [Getting Started](docs/GettingStarted.md) page.
This project only provides directions for installing on Ubuntu Linux, Arch Linux, and Mac OS X, although it shouldn't be too difficult to port to other operating systems.

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

This will install conan for you if you have not previously installed it. If you respond no to the setup script see the note under Setting up Conan for RoboJackets Firmware in [Getting Started](docs/GettingStarted.md) before continuing.

3) Build the project for the desired target. The `robot` target is the firmware for the MTrain.
The `kicker` target is for the kicker MCU to be uploaded to the MTrain.
The `clean` target deletes the build directories for both robot and kicker firmware.

```
$ make <TARGET>
```

We use Conan as our build system and have a simple `makefile` setup that invokes Conan. Conan in-turn invokes CMake.


## Testing

Firmware tests can be written and placed in `src/hw-test` with the name `test-<TESTNAME>.cpp` then compiled with `make robot-test-<TESTNAME>`

Generic firmware tests can be run with `make test-firmware`. (Still not completely sure what this does)


## Documentation

We use [Doxygen](www.doxygen.org) for documentation.  This allows us to convert specially-formatted comments within code files into a nifty website that lets us easily see how things are laid out.  Our compiled doxygen documentation can be found here:

http://robojackets.github.io/robocup-firmware/

Note: The doxygen documentation site above is updated automacally using circle-ci.  See our autoupdate-docs.sh file for more info.

## Contributing

Please see the [contributing page](docs/Contributing.md) before contributing.

## License

This project is licensed under the Apache License v2.0.  See the [LICENSE](LICENSE) file for more information.
