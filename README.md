# mTrain

![Build Status](https://github.com/RoboJackets/mtrain-firmware/workflows/CI/badge.svg)

The purpose of this project is to create a small, more capable microcontroller board. The hardware design files for this board can be found at [mtrain-pcb](https://github.com/RoboJackets/mtrain-pcb).


## Setup

It is recommended that you follow the instructions in the guide below to set up the repo. This was written for robocup-firmware's setup script but the process is identical to the setup script here in the mtrain-firmware repo under the util directory.

[Setup](https://github.com/RoboJackets/robocup-firmware/blob/master/doc/GettingStarted.md#setting-up-the-robocup-firmware-repository)

## Getting Started

For resources for beginners to embbedded development see the [Resources Page](https://github.com/RoboJackets/mtrain/wiki/Resources).


## Building and Programming

1) `make` to build all firmware
2) Rebuild tests and upload using JLink software:
    * For CPP tests: `make upload-{test name}` to rebuild and program a CPP test (eg. `make upload-blink`)
    * For C tests: `make upload-{test name}-c` to rebuild and program a C test (eg. `make upload-blink-c`)


## For Package Contributors
mTrain is also available as a conan package. Attached below is a guide intended for those who need would like to build and upload the a finished conan package for mtrain-firmware to services like bintray so that it will be usable in robocup-firmware.

[link](https://github.com/RoboJackets/robocup-firmware/blob/master/doc/Conan.md)
