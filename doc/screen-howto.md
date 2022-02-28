# How to Get Firmware Debug Messages

To view debugging messages from the robot firmware, make sure you have `screen` installed on your machine. (You can install the usual way using your package manager.) It also recommended to install `minicom` which allows you to view timestamps for serial messages as well as some other useful features. (See their [manual](https://wiki.emacinc.com/wiki/Getting_Started_With_Minicom) for more.)

## Step 1: Plug in the mTrain
Plug in the mTrain using a mini-USB cable. If you would like to flash the firmware, you will also need to plug it into the J-link (see [this document](https://github.com/RoboJackets/robocup-firmware/blob/master/doc/GettingStarted.md) for instructions).

## Step 2: Turn on the robot

## Step 3: Open `screen` (or `minicom`)
Run the program of your choice in the directory `/dev/ttyACM0`. If are using `screen`, you will have to run it *after* starting the robot/flashing the firmware. Minicom should automatically detect when the folder exists and the robot starts.

## Step 4: Read the debug messages
You should now be seeing debug messages in your terminal. You can also add timestamps to your messages if you are using `minicom` by pressing `Ctrl-A-N` simultaneously.
