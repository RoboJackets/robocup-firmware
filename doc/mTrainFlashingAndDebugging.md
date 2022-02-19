# mTrain Flashing

## Intro
The mTrain on boot runs the user program located at the predetermined memory address `0x0800000`. We flash code to this address using the J-Link tool in-order for the code to be executed by the mTrain.

## Installing J-Link Software
The J-Link software can be downloaded at https://www.segger.com/downloads/jlink/ for your operating system. Once you have installed it, you can run the command `jlinkexe` in your terminal and should see the following or newer:

```
SEGGER J-Link Commander V7.60h (Compiled Feb  9 2022 14:10:53)
DLL version V7.60h, compiled Feb  9 2022 14:10:46
```
Use `exit` to leave JLinkExe.

## Using the J-Link
Plug in the JLink unit to a usb port on your computer
Attach the ARM-JTAG-20-10 connector to the 20 pin connector side of the J-Link unit.

Attach the smaller 10 pin connector to the 10 pins on the top of the mTrain.

Plug in the mTrain mini usb connector and plug the other end to your computer.
TODO attach picture here of JLink header and where it connects on the mTrain

The mini usb powers the mTrain if its not receiving power from the board so it should light up after this.

## Flashing code to the mTrain
See the setup process [here](../README.md) if necessary.

To build it again run the following:
```
make
```

After building navigate to the top level of the mtrain firmware repo. Then navigate to the build location of the mtrain binaries:
```
cd build/bin/
```

Run J-LinkExe
```
JLinkExe
```

At this point you will get the same message as in the setting up J-Link software section. Then you need to connect to the processor via the following command
```
connect
```
continue hitting enter to use all the default options and you should get a wall of processor information and a message that you have connected.

To program the mtrain use:
```
loadbin ./test.bin 0x08000000
```

You should now see a progress bar for erasing flash and subsequently flashing memory. After this completes, you are done and can exit J-Link using `exit`. Restart the mTrain with the push button on top of the module and your code should now be running.
