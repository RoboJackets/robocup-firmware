## Firmware Overview
For a high level overview of robocup firmware see [here](Firmware.md)


## Command Line Basics
For a brief introduction to how to use the command line see [here](CommandLineBasics.md)


## Git Version Control Basics

Two good online learning games for git:
https://learngitbranching.js.org/?locale=en_US
https://github.com/git-game/git-game-v2

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

## Getting Started with C++ and Object Oriented Programming

In terms of understanding the usefulness of object oriented programming I recommend the Java concept tutorials although you will be briefly introduced to java code if find its a great conceptual starter.

In terms of a full C++ tutorial I recommend W3 schools C++ tutorials as they are easier to understand for beginners.
https://www.w3schools.com/cpp/

In terms of an in-depth tutorial of C++ I also recommend a Tour of C++ by Bjarne Stroustrup

## Flashing Code to the mTrain
For a brief introduction to how to use the command line see [here](FlashingMtrain.md)

## Working with Conan

For details on building packages with Conan see [here](Conan.md)

## Debugging
