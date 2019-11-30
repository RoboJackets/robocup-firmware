## Firmware Overview
For a high level overview of robocup firmware see [here](Firmware.md)


## Command Line Basics



## Setting up the robocup-firmware repository

### Linux (Tested on Ubuntu 18.04 LTS)
### Clone the repositories
git clone https://github.com/robojackets/mtrain-firmware
git clone https://github.com/robojackets/robocup-firmware

cd into the robocup-firmware directory
cd robocup-firmware
There are a few setup scripts in the util directory for installing required packages, setting up udev rules, etc.  Run `ubuntu-setup`, `arch-setup`, and `osx-setup` depending on your system

```
$ cd robocup-firmware
$ ./util/<SYSTEM>-setup
```

### Setting up Conan for RoboJackets Firmware
As part of the system setup script conan, the robojackets remote, and the required conan profile should have been setup.
Thus the steps for setting up conan are not necessary and are kept for reference when setting up on older branches.
NOTE: If you chose to not let the script overwrite your previous conan settings you will need to perform step 2 and 3 manually.
1) Install conan
pip install conan
2) Add the robojackets remote
conan remote add robojackets https://api.bintray.com/conan/robojackets/conan
3) cd into the mtrain-firmware repository and copy the file util/conan-config/profiles/armv7hf to ~/.conan/profiles

### Building robocup-firmware with Conan
cd into the robocup-firmware repository
make robot


## Getting started with C++ and Object Oriented Programming

## Flashing code to the mtrain?

## Working with Conan
For details on building packages with conan see [here] (Conan.md)
## Debugging
