# mTrain

The purpose of this project is to create a small, more capable microcontroller board. The hardware design files for this board can be found at [mtrain-pcb](https://github.com/RoboJackets/mtrain-pcb).


## Setup

It is recommended that you follow the instructions in the guide below to set up the repo. This was written for robocup-firmware's setup script but the process is identical to the setup script here in the mtrain-firmware repo under the util directory.

[Setup](https://github.com/RoboJackets/robocup-firmware/blob/master/doc/GettingStarted.md#setting-up-the-robocup-firmware-repository)

## Getting Started

For resources for beginners to embbedded development see the [Getting Started](https://robojackets.github.io/robocup-firmware/) Page for robocup-firmware.

For resources pertaining to the hardware of the mTrain including datasheets and schematics see the [Resources Page](doc/ExternalResources.md).


## Building and Programming

1) `make` to build all firmware
2) Rebuild tests and upload using JLink software:
    * For CPP tests: `make <TESTNAME>` to rebuild and program a CPP test (eg. `make blink`)
    * For C tests: `make <TESTNAME>-c` to rebuild and program a C test (eg. `make blink-c`)
