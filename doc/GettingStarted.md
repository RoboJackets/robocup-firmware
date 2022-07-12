# Getting Started

# Basics

## Command line basics
For a brief introduction to how to use the command line see [here](CommandLineBasics.md)


## Setting up the robocup-firmware repository
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


## Git version control
Please see [here](Git.md) for a brief tutorial on git as well as links to learning resources


## Contributing
Please see the [contributing page](Contributing.md) for general contributing guidelines.


## Getting started with C++ and object oriented programming
In terms of understanding the usefulness of object oriented programming I recommend the Java concept tutorials although you will be briefly introduced to java code if find its a great conceptual starter.

In terms of a full C++ tutorial I recommend W3 schools C++ tutorials as they are easier to understand for beginners.
https://www.w3schools.com/cpp/

In terms of an in-depth tutorial of C++ I also recommend a Tour of C++ by Bjarne Stroustrup

## GDB
For a quick intro to the basic GDB commands see [here](GDB.md)

For a interactive exercise for learning GDB see [here](GDBExercise.md)


# Advanced

## Firmware Overview
[Firmware Overview](Firmware.md)

[Kicker Details](Kicker.md)

[FPGA Details](FPGA.md)


## Working with the mTrain
For details on flashing code to the mTrain after compiling and debugging programs see [here](mTrainFlashingAndDebugging.md)


## Working with Conan
For details on building packages with Conan see [here](Conan.md)
