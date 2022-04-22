# GT RoboJackets RoboCup SSL - Firmware

The Georgia Tech RoboJackets team competes in the annual RoboCup Small Size League (SSL) competition.  This repository contains all of the firmware that we run on our robots. Also, check out our [2020 qualification video](https://www.youtube.com/watch?v=2MZREc9aj8k) to see our robots in action!

## Competition

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

The drivers themselves are defined in control/include/drivers.

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

1) Install WSL (Skip to Step 2 if using MacOS or Ubuntu)

Follow the steps outlined here: https://docs.microsoft.com/en-us/windows/wsl/install
Essentially, you'll open PowerShell as Administrator and type:

```
wsl --install -d Ubuntu
```

You should then see the ubuntu terminal application open in a separate window. It'll have you create a username and password, so make something memorable.

The path to the root folder using your file explorer is:
`C:\Users\<WindowsUsername>\AppData\Local\Packages\CanonicalGroupLimited.UbuntuonWindows_79rhkp1fndgsc\LocalState\rootfs`
This will be represented as a / in the terminal. You can get here quickly in the Ubuntu terminal by typing `cd /`

By default, when using WSL, you'll be in your home directory for WSL, so: 
`C:\Users\<WindowsUsername>\AppData\Local\Packages\CanonicalGroupLimited.UbuntuonWindows_79rhkp1fndgsc\LocalState\rootfs\home\<WSLUsername>`
This will be represented as a ~ in the terminal. You can get here quickly in the Ubuntu terminal by typing `cd ~`

Additionally, you'll notice you now have two new apps on your computer, those being WSL and Ubuntu. Using the Ubuntu app is recommended going forward because it'll place you in the home directory by default, but operating in both is much the same process. When you do the following steps, execute them from the ~ directory.

2) Clone the repository

```
git clone git://github.com/RoboJackets/robocup-firmware
```

3) Install the necessary software

There are a few setup scripts in the util directory for installing required packages, setting up udev rules, etc.  See `ubuntu-setup` and `macos-setup` for more info. If you are using WSL, see `wsl-setup` after using `ubuntu-setup`.

```
$ cd robocup-firmware
$ ./util/<SYSTEM>-setup
```

4) Build the project for the desired target. The `control` target is the firmware for the mTrain. The `kicker` target is for the kicker MCU to be uploaded to the MTrain. The `clean` target deletes the build directories for both robot and kicker firmware.

```
$ make <TARGET>
```

Generally, the flashing process goes by you executing commands in the following order

```
$ make clean
$ make kicker
$ make
```

If you're on Windows, and after executing make it states that JLink.exe cannot be found, make sure you ran

```
$ ./util/wsl-setup
```

Then, it should work normally.

## Testing

Firmware tests can be written and placed in `control/test` with the name `<TESTNAME>.cpp` then compiled with `make <TESTNAME>`

## Documentation

We use [Doxygen](https://www.doxygen.nl/index.html) for documentation.  This allows us to convert specially-formatted comments within code files into a nifty website that lets us easily see how things are laid out.  Our compiled doxygen documentation can be found by doing:

```
$ make docs
```

## Contributing

Please see the [contributing page](doc/Contributing.md) before contributing.

## License

This project is licensed under the Apache License v2.0.  See the [LICENSE](LICENSE) file for more information.
