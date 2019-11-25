This document was originally written Fall of 2019 after the mtrain implementation. The latest relevant branch was comp2019. Knowing robocup, this won't be updated to the latest version of firmware, but hopefully it will still describe most everything. - Joe Neiger

# FPGA

This document will lightly touch the FPGA verilog code, motor physics, and the communication protocol. I (Joe) have not personally worked on the FPGA code so this is all from my reading and debugging of the code over the years.

## FPGA Code

On the high level, the FPGA takes in motor speed commands over SPI, communicates specific phases high and low to the motor driver, reads the encoder values, and returns the encoder values and delta time over SPI.

One big thing to keep in mind as you work with the FPGA, from the mtrain side, there is a built in watchdog in the FPGA. You must command motor values consistently or toggle the motors on->off->on otherwise the watchdog will trigger.

## Motor Lowdown

RoboCup uses BLDC motors for all their movement needs. BLDC (Brushless DC) basically means that instead of using brushes to swap the positive and negative magnetic poles in the motor, we do it electrically. There are 3 phases evenly distributed around the physical device. You can rotate a magnet through a full circle by turning on the attraction side in front of the magnet, turning on the repellent side behind the magnet, and letting the third one just sit without a field. This will allow for rotations of the magnet itself. These three phases are called A, B, and C. To actually cause the motor to rotate, you need to know where the magnet is located in it's rotation. We use hall effect sensors for this purpose. They are very course grain position sensors who's output is which 60 degree quadrant you are in. This is perfect for interfacing with the 3 phases. A simple state machine can be built that says for each quadrant, a specific combination of phases should be turned on and off. To get the speed correct, you can change the voltage going to the motors. This is done using PWM (Pulse Width Modulation). One can easily imagine this by turning off and on the power very quickly to the motor. 

## Communication Protocol

The mtrain <-> fpga communciation through SPI is built up using a command based system. The first byte sent represents the type of data.

| Command | Hex | Description |
|---|---|---|
| Disable Motors | 0x30 | Disables all motors |
| Enable Motors | 0xB0 | Enables all motors |
| Read Encoders Write Vel | 0x80 | Writes each of the 5 duty cycles out to the FPGA while reading encoders and dt at the same time |
| Read Encoders | 0x91 | Reads back all 4 encoders and the dt |
| Read Halls    | 0x92 | Reads back all 5 hall effect sensors |
| Read Duty     | 0x93 | Reads back all 5 currently  commanded duty cycles |
| Read Hash Pt 1| 0x94 | Reads the second half of the git hash of the fpga |
| Read Hash Pt 2| 0x95 | Reads the first half of the git hash of the fpga |
| Check DRV     | 0x96 | Reads the config of the DRV3303 gate drivers |

### Disable/Enable Motor Format

| | |
|-|-|
| Send   | 0x30/0xB0 |
| Receive| Status    |

Enables or disables the motors based on the value.

Note: Toggle on->off->on resets the watchdog

### Read Encoders Write vel

| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x80  | Duty cycle #1 | Duty cycle #2 |Duty cycle #3 |Duty cycle #4 | Duty cycle #5 |
| Receive | Status| Delta Enc #1  | Delta Enc #2  |Delta Enc #3  |Delta Enc #4  | Delta time    |

Duty cycle is a 11 bit number in signed magnitude form (-511 to 511). The lower byte of the 16 bit number is sent, then the higher byte of the 16 bit number is sent. On the receiving side the high byte is sent first, then the lower byte. The Delta encoder values are signed 16 bit numbers.

Note: Writing this command resets the watchdog

### Read Encoders

| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x91   | 0x0000       | 0x0000      | 0x0000      | 0x0000      | 0x0000     |
| Receive | Status | Delta Enc #1 |Delta Enc #2 |Delta Enc #3 |Delta Enc #4 | Delta time |


On the receiving side the high byte is sent first, then the lower byte. The Delta encoder values are signed 16 bit numbers.

### Read Halls

| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x92   | 0x00    | 0x00    | 0x00    | 0x00    | 0x00    |
| Receive | Status | Hall #1 | Hall #2 | Hall #3 | Hall #4 | Hall #5 |

### Read Duty Cycle

| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x93   | 0x0000         | 0x0000         | 0x0000         | 0x0000         | 0x0000         |
| Receive | Status | Duty Cycles #1 | Duty Cycles #2 | Duty Cycles #3 | Duty Cycles #4 | Duty Cycles #5 |

Duty cycle is a 11 bit number in signed magnitude form (-511 to 511). On the receiving side the high byte is sent first, then the lower byte.

### Git Hash 1/2


| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x94   | 0x00   | 0x00   | .... | 0x00   | 0x00   |
| Receive | Status | Char20 | Char19 | .... | Char12 | Char11 |


| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x95   | 0x00   | 0x00   | .... | 0x00   | 0x00  |
| Receive | Status | Char10 | Char09 | .... | Char01 | Dirty |

If enabled returns the git hash of the fpga firmware. Care must be taken to flip the returned list of characters to get the correct git hash. Additionally, the last character back is `0x01` if the fpga firmware is dirty.

### Check DRV

| | | | | | | |
|-|-|-|-|-|-|-|
| Send    | 0x96   | 0x00 | 0x00 | 0x00 | 0x00 |....|
| Receive | Status | Nib1 | Nib0 | 0x00 | Nib2 |....|

For 5 Nibble Set transactions (20 8 bit spi transactions).

Each nibble has a very specific output given by the table (MSB -> LSB). See DRV data sheet for definitions.

| | | | | |
|-|-|-|-|-|
| Nib2 | GVDD_OV  | FAULT    | GVDD_UV  | PVDD_UV  |
| Nib1 | OTSD     | OTW      | FETHA_OC | FETLA_OC |
| Nib0 | FETHB_OC | FETLB_OC | FETHC_OC | FETLC_OC |

### Return Status

For every single packet, the first byte returned in the packet type class is the status. (MSB -> LSB)

| | | | |
|-|-|-|-|
| System Ready | Watchdog Trigger | Motors Enabled | Motors Has Error[4..0] |

System ready is true when the system has finished the boot up configuration.

Watchdog Trigger is true when the watchdog times out

Motors Enabled is true when the commanded duty cycles will actually cause movement in the motors.

Motors Has Error is a 5 bit field that is true when that specific motor has a hall fault or the hall is not connected.

## DT Note

The delta time at the end of most of the packets is very special and has an extremely crazy conversion. This snippet of code will do everything for you. Copy this wherever it is used to keep the documentation near it.

```
    /*
     * The time since the last update is derived with the value of
     * WATCHDOG_TIMER_CLK_WIDTH in robocup.v
     *
     * The last encoder reading (5th one) from the FPGA is the watchdog
     * timer's tick since the last SPI transfer.
     *
     * Multiply the received tick count by:
     *     (1/18.432) * 2 * (2^WATCHDOG_TIMER_CLK_WIDTH)
     *
     * This will give you the duration since the last SPI transfer in
     * microseconds (us).
     *
     * For example, if WATCHDOG_TIMER_CLK_WIDTH = 6, here's how you would
     * convert into time assuming the fpga returned a reading of 1265 ticks:
     *     time_in_us = [ 1265 * (1/18.432) * 2 * (2^6) ] = 8784.7us
     *
     * The precision would be in increments of the multiplier. For
     * this example, that is:
     *     time_precision = 6.94us
     *
     */
    float dt = static_cast<float>(encDeltas[4]) * (1 / 18.432e6) * 2 * 128;
```