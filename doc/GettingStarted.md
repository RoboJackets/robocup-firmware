# Basics
## Command Line Basics
For a brief introduction to how to use the command line see [here](CommandLineBasics.md)


## Setting Up the robocup-firmware Repository
### Clone the repositories
```
git clone https://github.com/robojackets/mtrain-firmware
git clone https://github.com/robojackets/robocup-firmware
```

There are a few setup scripts in the util directory for installing required packages, setting up udev rules, etc.  Run `ubuntu-setup`, `arch-setup`, and `osx-setup` depending on your system.
```
cd robocup-firmware
./util/<SYSTEM>-setup
```


### Setting Up Conan for robocup-firmware
As part of the system setup script conan, the robojackets remote, and the required conan profile should have been setup.
Thus the steps for setting up conan are not necessary and are kept for reference when setting up on older branches.

NOTE: If you chose to not let the script overwrite your previous conan settings you will need to perform step 2 and 3 manually.

1) Install conan
```
pip install conan
```

2) Add the robojackets remote
```
conan remote add robojackets https://api.bintray.com/conan/robojackets/conan
```

3) cd into the mtrain-firmware repository and copy the file util/conan-config/profiles/armv7hf to ~/.conan/profiles

### Building robocup-firmware with Conan
cd into the robocup-firmware repository
```
make robot
```


## Git Version Control
Please see [here](doc/Git.md) for a brief tutorial on git as well as links to learning resources


## Contributing
Please see the [contributing page](doc/Contributing.md) for general contributing guidelines.


## Getting Started with C++ and Object Oriented Programming
In terms of understanding the usefulness of object oriented programming I recommend the Java concept tutorials although you will be briefly introduced to java code if find its a great conceptual starter.

In terms of a full C++ tutorial I recommend W3 schools C++ tutorials as they are easier to understand for beginners.
https://www.w3schools.com/cpp/

In terms of an in-depth tutorial of C++ I also recommend a Tour of C++ by Bjarne Stroustrup

## Debugging
For a quick intro to the basic GDB commands see [here](GDB.md)

For a interactive exercise for learning GDB see [here](GDBExercise.md)


# Advanced

## Firmware Overview
[Firmware Overview](Firmware.md)

[Kicker Details](Kicker.md)

[FPGA Details](FPGA.md)


## Flashing Code to the mTrain
For details on flashing code to the mtrain after compiling see [here](FlashingMtrain.md)


## Working with Conan
For details on building packages with Conan see [here](Conan.md)
