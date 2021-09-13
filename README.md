# GT RoboJackets RoboCup SSL - Firmware

The Georgia Tech RoboJackets team competes in the annual RoboCup Small Size League (SSL) competition.  This repository contains all of the firmware that we run on our robots. More information on how our software works can be found on our [documentation page](http://robojackets.github.io/robocup-firmware/), our [wiki](http://wiki.robojackets.org/w/RoboCup_Software) or on our [website](http://www.robojackets.org/).
Also, check out our [2020 qualification video](https://www.youtube.com/watch?v=2MZREc9aj8k) to see our robots in action!

## The Competition

The soccer game is played between two teams of six robots each on a field with overhead cameras.  The field vision computer processes images from the cameras and sends out (x,y) coordinates of the ball and the robots.  Each team has a laptop that accepts the data from the vision computer and uses it to strategize, then send commands to robots on the field.

The official [RoboCup site](https://ssl.robocup.org/) has more information on the competition.

# Getting Started
If you are a new member of RoboCup Electrical or Software and are interested in getting involved with firmware it is highly recommend you start with the [getting started](doc/GettingStarted.md) pages.

## Project Layout
For a high level overview of architecture of robocup-firmware see [here](doc/Firmware.md)

### /
The top level folder contains the firmware code for the different targets that are compiled for the control board, FPGA, IMU, and radio.

### control/
Contains the firmware code for the main program run on the mtrain. This controls the overall function of the robot and the interface with various peripheral devices, such as IMU, FPGA, and the radio

Modules are fully independent sections of code that take a well defined input (if applicable), do some action, then produce a well defined output (if applicable). For example, the kicker module takes kicker commands (input), directly interfaces with the kicker and does the communication and unit translation (action), and produces breakbeam and voltage to the rest of the firmware (output).

Modules are a class following the interface found in generic_module. The module specifies how often an action should run. At that specific frequency, the entry function will be called to do the action part of the module. The inputs and outputs are given through shared structures passed in the constructor.

The drivers themselves are defined in cotrol/include/drivers.

### control/build/
Compiled binaries output from the build system for the mtrain connected to the control board get stored here in the bin/ directory as well as other build results.

### kicker/
The kicker folder contains the firmware code for the different targets that are compiled for the kicker board.
Details on the architecture of the kicker firmware code can be found [here](doc/Kicker.md)

### kicker/build/
Compiled binaries output from the build system for the ATMega on the kicker board get stored here in the bin/ directory as well as other build results.

### fpga/
Contains the Verilog code to be programmed to the FPGA for motor control.
Details on the architecture of the FPGA firmware code can be found [here](doc/FPGA.md)


## Setup
Below is a quick guide to getting this RoboCup project setup to build on your computer. If you are a robocup member planning on developing both mtrain and robocup firmware it is highly recommended that you  follow the Firmware [Getting Started](doc/GettingStarted.md) page.
This project only provides directions for installing on Ubuntu Linux, Windows Subsystem for Linux (WSL), and macOS.

1) Clone the repository

```
git clone git://github.com/RoboJackets/robocup-firmware
```

2) Install the necessary software

There are a few setup scripts in the util directory for installing required packages, setting up udev rules, etc.  See `ubuntu-setup` and `macos-setup` for more info. If you are using WSL, see `wsl-setup` after using `ubuntu-setup`.

```
$ cd robocup-firmware
$ ./util/<SYSTEM>-setup
```

3) Build the project for the desired target. The `control` target is the firmware for the mTrain.
The `kicker` target is for the kicker MCU to be uploaded to the MTrain.
The `clean` target deletes the build directories for both robot and kicker firmware.

```
$ make <TARGET>
```


## Testing

Firmware tests can be written and placed in `control/test` with the name `<TESTNAME>.cpp` then compiled with `make <TESTNAME>`

## Documentation

We use [Doxygen](https://www.doxygen.nl/index.html) for documentation.  This allows us to convert specially-formatted comments within code files into a nifty website that lets us easily see how things are laid out.  Our compiled doxygen documentation can be found here:

http://robojackets.github.io/robocup-firmware/

Note: The doxygen documentation site above is updated automacally using circle-ci.  See our autoupdate-docs.sh file for more info.

## Contributing

Please see the [contributing page](doc/Contributing.md) before contributing.

## License

This project is licensed under the Apache License v2.0.  See the [LICENSE](LICENSE) file for more information.
