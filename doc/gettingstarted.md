## Setting up the robocup-firmware repository

### Linux (Tested on Ubuntu 18.04 LTS)
### Clone the repositories
git clone https://github.com/robojackets/mtrain-firmware
git clone https://github.com/robojackets/robocup-firmware

### Installing compilers
sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt update
sudo apt install gcc-arm-embedded

## Setting up Conan for RoboJackets Firmware
Install conan
pip install conan
conan remote add robojackets https://api.bintray.com/conan/robojackets/conan
cd into the mtrain-firmware repository and copy the file armv7hf to ~/.conan/profiles
Running Conan
cd into the robocup-firmware repository
make robot
