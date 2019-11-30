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

## Setting up Conan for RoboJackets Firmware
Install conan
pip install conan
conan remote add robojackets https://api.bintray.com/conan/robojackets/conan
cd into the mtrain-firmware repository and copy the file armv7hf to ~/.conan/profiles
Running Conan
cd into the robocup-firmware repository
make robot
