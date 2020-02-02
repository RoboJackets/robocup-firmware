# Flashing code to the mtrain
## Intro
The mTrain on boot runs the user program located at the predetermined memory address 0x0800000
We flash code to this address using the J-Link in-order to be executed.

## Installing J-Link Software
The J-Link software can be downloaded at:
https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack
For Ubuntu you want to download the file labeled as 'J-Link Software and Documentation pack for Linux, DEB installer, 64-bit'

Provided that file was stored in your downloads folder you can use the below to install the package:
```
sudo dpkg -i ~/Downloads/JLink_Linux_V656a_x86_64.deb
sudo apt install -f
```

NOTE The name of the exact file may be slightly different depending on what version you downloaded so copying and pasting the above may not work.

After installing the J-Link software package please restart your terminal emulator and try to tab complete and open the program JLinkExe. If you are successful you will be greeted by the following:

```
SEGGER J-Link Commander V6.56 (Compiled Nov 22 2019 17:14:15)
DLL version V6.56, compiled Nov 22 2019 17:14:02
```

Followed by a can not connect error if the J-Link is not connected. Close JLinkExe with exit.

## Setup The JLink Unit
Plug in the JLink unit to a usb port on your computer
Attach the ARM-JTAG-20-10 connector to the 20 pin connector side of the J-Link unit.

Attach the smaller 10 pin connector to the 10 pins on the top of the mTrain.

Plug in the mTrain mini usb connector and plug the other end to your computer.
TODO attach picture here of JLink header and where it connects on the mtrain

The mini usb powers the mTrain if its not receiving power from the board so it should light up after this.

## Flashing code to the mTrain
If you have not already setup robocup-firmware to build do that now (following the guide in getting started).
To build it again run the following:
```
make all
```

After building navigate to the top level of the robojackets firmware repo. Then navigate to the build location of the robocup binaries:
```
cd robot/build/bin/
```

Run J-LinkExe
```
JLinkExe
```

At this point you will get the same message as in the setting up jlink software section. Then you need to connect to the processor via the following command
```
connect
```
continue hitting enter to use all the default options and you should get a wall of processor information and a message that you have connected.

To program the mtrain use:
```
loadbin ./control.bin 0x08000000
```

You should now see a progress bar for erasing flash and subsequently flashing memory. After this completes your are done and can exit J-Link. Restart the mTrain with the push button on top of the module and your code should now be running.


# Debugging mTrain
## Whats different than using GDB as usual?
Using GDB for debugging with RoboCup Firmware is different than debugging a program on your computer. Normally when start up an instance of GDB, you either start it with a binary name or use file after starting along with a file path to a binary to debug. If a binary is compiled with debug symbols (eg using the -g flag on gcc) the binary will have a symbol table which stores the human readable symbol names for memory locations (variables). Of course in addition to loading the symbol table if it exist gdb also loads the program into your memory and then waits for your to issue run before executing any instructions.

All of the above assumes we are running the program on the same computer we are running gdb. With RoboCup firmware this is not the case. Our program is being executed by the mTrain so we want to debug the code as it runs on the mTrain, not our local system. To do this we setup a gdbserver on the J-Link itself, which will preform the functions gdb did on our local system for the program running on the mtrain (eg starting and stopping the program as it encounters certain memory addresses like breakpoints). However the binary (.bin) we flash to the robot does not contain debug symbols, only the .elf does. The gdb server running on the J-Link does not have any idea what the symbol table contains, just at what memory addresses to stop. Instead we boot a local instance of gdb with the symbol table by loading the .elf and then connect to the gdb server running on the J-Link to control GDB (issue breakpoints, print a variable's value, etc).

## Running GDB on the mTrain
1. Follow the steps outlined above for flashing code to the mTrain expect running make clean before you start and running make debug instead of make all
2. Run the following to start the gdb server on the J-Link:
```
JLinkGDBServer -select USB -device STM32F769NI -endian little -if JTAG -speed auto -noLocalhostOnly
```
3. In another terminal you will need to run the version provided with the GnuArmNoneToolchainInstaller package that was install as part of the firmware setup script, with the file to load being the control.elf we built. When run from the bin folder in ./robot/build/ this would look like:
```
~/.conan/data/GnuArmNoneToolchainInstaller/0.1.0/robojackets/testing/build/<user-specific-hash>/7.3_Linux/gcc-arm-none-eabi-7-2018-q2-update/bin/arm-none-eabi-gdb ./control.elf
```
Replacing the <user-specific-hash> section with whatever it is your on your system. It may be easier to navigate to this directory on your system tab completing whatever you dont know from this since there should only be one folder and the use pwd and copy the path.

4. After opening GDB we connect to the gdbserver with:
```
target remote localhost:2331
```

5. Set whatever breakpoints you would like and start executing the program with continue (not run)

## Restarting the program
Usually we would use run to reload the program but when using GDB server the process is slightly different(note we didnt use run to start the program either). Instead we issue the following two commands to restart the program from the beginning:
```
monitor reset
load
```
