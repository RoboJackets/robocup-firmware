# mTrain

The purpose of this project is to create a small, more capable microcontroller board. The hardware design files for this board can be found at [mtrain-pcb](https://github.com/RoboJackets/mtrain-pcb).

## Setup
Below is a quick guide to getting this mTrain project setup to build on your computer. This project only provides directions for installing on Ubuntu Linux, Windows Subsystem for Linux (WSL), and macOS.

1) Clone the repository

```
git clone https://github.com/RoboJackets/mtrain-firmware.git
```

2) Install the necessary software

There are a few setup scripts in the util directory for installing required packages.  Run `ubuntu-setup` or `macos-setup` accordingly. If you are using WSL, run `wsl-setup` after using `ubuntu-setup`.

```
$ cd mtrain-firmware
$ ./util/<SYSTEM>-setup
```
3) Build the project for the desired target. The default target builds only the firmware.

```
$ make
```

## Testing

Firmware tests can be written and placed in `tests/cpp` with the name `<TESTNAME>.cpp` then compiled with `make <TESTNAME>`

## Documentation

We use [Doxygen](https://www.doxygen.nl/index.html) for documentation.  This allows us to convert specially-formatted comments within code files into a nifty website that lets us easily see how things are laid out.  Our compiled doxygen documentation can be found by doing:

```
$ make docs
```

## Getting Started

For resources pertaining to the hardware of the mTrain including datasheets and schematics see the [Resources Page](doc/ExternalResources.md).

## License

This project is licensed under the Apache License v2.0.  See the [LICENSE](LICENSE) file for more information
