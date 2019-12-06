## Intro
The mTrain on boot runs the user program located at the predetermined memory address 0x0800000
We flash code to this address using the J-Link in-order to be executed.

## Installing J-Link Software
The J-Link software can be downloaded at:
https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack
For Ubuntu you want to download the file labeled as 'J-Link Software and Documentation pack for Linux, DEB installer, 64-bit'

Provided that file was stored in your downloads folder you can use the below to install the package:
```
sudo dpkg -i ~/downloads/JLink_Linux_V656a_x86_64.deb
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

## Flashing Code to the mTrain
If you have not already setup robocup-firmware to build do that now (following the guide in getting started).
To build it again run the following:
```
make robot
```

After building navigate to the top level of the robojackets firmware repo. Then navigate to the build location of the robocup binaries:
```
cd robot/build/bin/
```

Run J-LinkExe
```
JLinkExe
```

At this point you will get the same message as in the setting up jlink software section followed by a wall of information on the processor. If you didnt get a wall of processor information the J-Link did not auto connect and thus you will need to connect via 'connect' and hitting enter to use all the default options.

At this point you should get a message that you have connected.

To program the radio use:
```
loadbin ./control.elf 0x08000000
```

You should now see a progress bar for erasing flash and subsequently flashing memory. After this completes your are done and can exit J-Link. Restart the mTrain with the push button on top of the module and your code should now be running.
