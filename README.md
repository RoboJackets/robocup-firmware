# mTrain

The purpose of this project is to create a small, more capable microcontroller board. The hardware design files for this board can be found at [mtrain-pcb](https://github.com/RoboJackets/mtrain-pcb).


For some help getting started see the [Resources Page](https://github.com/RoboJackets/mtrain/wiki/Resources) of the wiki


## Basic Setup

1) Install neccesary tools:
    * Install `conan` through pip (Python 3)
2) Setup conan:
    * `conan remote add robojackets https://api.bintray.com/conan/robojackets/conan`
3) Install [J-Link Software and Documentation Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) (for programming and debugging)

## Building and Programming

1) `make` to build all firmware
2) Rebuild tests and upload using JLink software:
    * For CPP tests: `make upload-{test name}` to rebuild and program a CPP test (eg. `make upload-blink`)
    * For C tests: `make upload-{test name}-c` to rebuild and program a C test (eg. `make upload-blink-c`)

TODO: better setup for conan profile

TODO: add the rest of the readme
