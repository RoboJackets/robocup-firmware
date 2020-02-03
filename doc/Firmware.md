This document was originally written Fall of 2019 after the mtrain implementation. The latest relevant branch was comp2019. Knowing robocup, this won't be updated to the latest version of firmware, but hopefully it will still describe most everything. - Joe Neiger

# Robocup Firmware

The repo is split into three big sections: kicker, fpga, and control. Kicker is the firmware that sits on the kicker board. FPGA is the verilog for the FPGA. Control is the code that runs on the mtrain (not including the HAL).

Each section will be described from a high level overview. Next, the interactions between the sections will be described. Secondary documents will describe each section in detail along with future implementation setups. Finally, the normal use document will describe LED codes as well as common problems.

## Kicker Firmware

The kicker firmware deals with controlling the kicks. It tells the circuit to charge the capacitors, stop charging, and finally release the charge through the solenoids. It also checks for ball sense. The device has two modes, debug and standard. Debug allows the control of the charging and kicks through the buttons on the side of the board. Standard mode takes commands from mtrain over SPI acting as a slave. Important data such as ball sense and current voltage is passed back up to mtrain to send to soccer.

Kicks are controlled through toggling the current flow from the capacitors through the solenoid for a certain amount of time. It is unknown what the relationship between open current time and speed of ball kick is.

Note: Very special care is taken to make sure that the device is safe. Safe in this case means that it won't accidentally kick at the wrong time AND that it wont discharge and charge at the same time. This is a high voltage, high current device.

[Kicker Details](Kicker.md)

## FPGA Firmware

In short, the FPGA deals with all things motors. It is given duty cycle, forwards that to each of the motors, and returns what the current wheel speed is. In the future (tm), there is hope to also get the current of each motor as well. The FPGA communicates with mtrain over SPI as well.

Note: Motors are expensive so please be very careful when changing the code. It's very easy to make a mistake and fry one. (Totally hasn't happened before, nope, not at all)

[FPGA Details](FPGA.md)

## Robot Firmware

Robot firmware is all the code that runs on the mtrain. This is basically everything else. In general, this firmware takes commands from soccer over WiFi, interprets those commands and forwards them to the motors and kicker. Additionally, data is returned to soccer over WiFi to inform soccer of critical sensor info.

More specifically, kick commands and movement commands are sent to the radio. The firmware polls the radio for the raw data and sends it to the kicker (for kick commands) and to the motion control module (for movement commands). The motion control module puts out duty cycle commands that are forwarded to the FPGA. Wheel speed and ball sense information is taken from the FPGA and kicker and sent back through the radio to soccer. LED's are toggled on the board to describe operations.
