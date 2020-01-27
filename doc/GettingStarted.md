# Basics

## Command line basics
For a brief introduction to how to use the command line see [here](CommandLineBasics.md)


## Setting up the robocup-firmware repository
1) Clone the repositories
```
git clone https://github.com/robojackets/mtrain-firmware
git clone https://github.com/robojackets/robocup-firmware
```

2) Run the setup script
There are a few setup scripts in the util directory for installing required packages, setting up udev rules, etc.  Run `ubuntu-setup`, `arch-setup`, and `osx-setup` depending on your system.
```
cd robocup-firmware
./util/<SYSTEM>-setup
```

3) Compile robocup-firmware
make sure you are at the top level of robocup-firmware and run make
```
make robot
make
```


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
