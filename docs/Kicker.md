This document was originally written Fall of 2019 after the mtrain implementation. The latest relevant branch was comp2019. Knowing robocup, this won't be updated to the latest version of firmware, but hopefully it will still describe most everything. - Joe Neiger

# Kicker Firmware

This document will go through the two modes of operation, standard and debug, and give full details on other specifics of operation, such as communication protocol, voltage reading, SPI operation, kick timings.

## Standard

Standard mode takes commands from mtrain, acts on those commands, and returns the latest sensor data. This loop acts continuously.

 1. A kick command will be sent to the kicker from mtrain over SPI.
 2. The attiny will respond to this command by filling the global command structure with the given data. This includes kick type, power, and whether it is safe to charge.
 3. The current voltage on the caps will be read.
 4. If there is still room to charge to a higher voltage and we are safe to charge, we will do so.
 5. If we are given a kick or chip command, we will then execute that kick.
 6. Ball sense is read to see if a ball is breaking line of sight between the LED and photo resistor.
 7. The current voltage and ball sense is sent back to the mtrain.

## Debug

Debug mode is special. All actions are fully controlled by the buttons on the side. Only voltage reading and ball sense still occur automatically.

The charge button toggles the safe to charge variable which allows the device to charge up to the max safe charge level controlled by software.

The kick button forces a kick immediately, discharging the caps through the kicker solenoid.

The chip button forces a chip immediately, discharging the caps through the chipper solenoid.

## Command Protocol

See `kicker_commands.h`

In the old'en days, aka Spring of 2019 and before, we used to send multiple SPI transactions to the kicker to fully describe out commands. This was very inefficient due to the lag associated with the attiny processing the transition. It was at least 100 ms delay before another transaction can occur.

This new protocol was built to send the entire packet in a single byte. Each bit represents a different thing. 

### mtrain -> kicker

The lowest 4 bits (0 - 3) represent the power of the kick. 0 is min power, 15 is max power.

The next bit (4) represents whether the kicker can safely charge the caps or not. 0 is don't charge, 1 is charge

The next two bits (5 - 6) represent the type of kick activation. Specifically, whether you want to kick immediately, kick on breakbeam, or cancel a kick on breakbeam. 0b00 is do nothing, 0b01 is kick on breakbeam, 0b10 is kick immediately, 0b11 is to cancel a kick on breakbeam. Kick in this case means a kick or chip.

The most significant bit is what type of kick action is being taken. 0 is a linear kick on the ground, 1 is a chip.

### kicker -> mtrain

For the return from the kicker to mtrain, the only data that needs to go back is whether the breakbeam has tripped and the current voltage of the kicker. This allows for a very simple protocol.

The lowest 7 bits (0 - 6) define the voltage. The voltage is in units of adc_lsb/2. So `packet_voltage = adc_voltage / 2`. Multiple the packet voltage by 2 to get back to the adc lsb unit scale. This very roughly correlates to voltage.

The most significant bit is whether the break beam has tripped. 0 is not tripped (no ball), 1 is tripped (has ball)

## SPI Communication

SPI as a protocol is interesting due to the syncronice method of data transfer. Each side has a letter filled with information and they send them to each other at the same time. Due to this, it is not possible to react to a command coming in until the next message transfer.

AVR on this specific device uses an interrupt to tell the user that a SPI transaction occurred (`SPI_STC_vect`). This interrupt fires after the entire byte has been transferred and is now held in `SPDR`. We pull this byte out and set a global command struct to hold the corresponding data. Interrupt lengths must be very short otherwise the processor will never actually run the normal code.

At the same time we receive that byte, the processor automatically sends whatever data used to be in `SPDR` to the other device. It is for this reason we continuously update that with the latest data. The kicker doesn't control the time this SPI transaction occurs so it must always be ready.

Note: When acting on the `SPDR` register, never operate on the register (eg `SPDR & 0x2`), always fully copy the data over. Between two subsequent lines, this value can change due to the interrupt firing again. 

## Voltage Reading

The attiny has a built in analog to digital converter. This takes the analog voltage out the physical trace and converts it to a unitless number. This number is 0 - 255 which corresponds to 0V - Vcc on the physical line. Every 1000 iterations, the device reads the adc and saves it into the current global voltage variable. It is every 1000 iterations because there is a short lag time before starting the adc reading and actually getting the result. This lags the global loop causing a potential slowdown. Additionally, it's not that important that it is updated more than every 10 ms anyways.

Note: Based on the current resistor values on the board, the output of the adc very roughly correspond to the voltage of the board in volts. AKA 200 lsb out of the adc is just about 200 V on the high side

## The Kick Itself

For the kick, the idea is to allow the pathway between the caps and the solenoid to stay open for a specific amount of time, allowing current to flow through and cause the kick.

Since this is such a specific time, we use a timer interrupt to make sure it's consistent. A timer interrupt, once enabled, calls a function (`TIMER0_COMPA_vect`) every X amount of time. When a kick is supposed to happen, we start the timer, and after it has finished, we stop the timer.

Inside this interrupt we must be very careful. We do not want to be charging while the channel is open. This forces us to create 4 phases to the kick. The first phase is when we stop the charging, the second state is when we start the kick, the third state is when we end the kick, the fourth state we start charging again. Each phase has a specific timing, not just the kick itself. This is because the time scale we are seeing at this level is close enough to the regime where we must account for the transitions of the charging circuit as well as the transistors that control the free flow into the solenoids.

## Ball Sense

Ball sense filters the output of the sensor to clear false positives. It must be the same value for X amount of time to force the measurement to shift from `has_ball` to `not_has_ball` and back. More advanced and creative filtering techniques can be used, but since the kicker only talks to mtrain at 25-100 hz, it is not needed as the bulk of the delay is on the communication side, not the filter side.