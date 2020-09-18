.. _flashing

Flashing Your Microcontroller
*****************************

Overview
========
Once you have built your application, you will need to flash the resulting
executable to your microcontroller. There are several methods of doing this
as described in the following sections. 

STM32CubeProgrammer
===================
STM32CubeProgrammer (STM32CubeProg) is an all-in-one multi-OS software tool 
for programming STM32 products. It comes with both a GUI and CLI, and it 
supports reading, writing and verifying device memory through both the debug 
interface (JTAG and SWD) and the bootloader interface (UART, USB DFU, I2C, 
SPI, and CAN). You can download STM32CubeProgrammer for free from 
`ST Microelectronics`_. STM32CubeProg is the most flexible option for 
flashing your application, and is the recommended programming method. 
Review the `STM32CubeProgrammer Users Manual`_ for a detailed reference on 
how to use the application.

Installation
------------

mJackets Flash Script
=====================
Provided with the mJackets API is a multi-os compatible python cli 
script for programming STM32 microcontrollers. 


.. _ST Microelectronics: https://www.st.com/en/development-tools/stm32cubeprog.html
.. _STM32CubeProgrammer Users Manual: https://www.st.com/resource/en/user_manual/dm00403500-stm32cubeprogrammer-software-description-stmicroelectronics.pdf