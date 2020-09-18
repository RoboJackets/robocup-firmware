.. _debugging:

Debugging Guide
###############

Being able to debug your code is pretty much an essential skill for developing 
embedded firmware. While `printf` statements are a perfectly valid way of 
debugging, modern debugging tools exist to supercharge your bug tracking. Some
features of modern debuggers include setting breakpoints, stepping through code,
watching variables, and much more. 

Any modern debugging tool that supports ARM Cortex-M microcontrollers will probably 
work for debugging STM32 chips, but GDB is the officially supported debugging engine
for mJackets. GDB, the GNU Project debugger, allows you to see what is going on 
'inside' another program while it executes -- or what another program was doing at the
moment it crashed. GDB can do four main kinds of things (plus other things in support 
of these) to help you catch bugs in the act:

1. Start your program, specifying anything that might affect its behavior.
2. Make your program stop on specified conditions.
3. Examine what has happened, when your program has stopped.
4. Change things in your program, so you can experiment with correcting the effects of one bug and go on to learn about another.

The following sections will detail getting GBD set up with your development environment.

Install Required Tools
**********************
Several tools are required to debug STM32 microcontrollers:
  - ARM GCC Toolchain (`https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>`_) - provides arm-none-eabi-gdb and related tools
  - J-Link Software Tools (https://www.segger.com/downloads/jlink) - provides the J-Link GDB Server for J-Link based debuggers

.. _installing_jlink_software:

Installing J-Link Software
==========================

The J-Link Software Pack can be downloaded at: <https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack> . 

  - For Ubuntu you want to download the file labeled as 'J-Link Software and Documentation pack for Linux, DEB installer, 64-bit'
  - For Windows you want to download the file labeled as 'J-Link Software and Documentation pack for Windows'

Provided that file was stored in your downloads folder you can use the below to install the package:

.. code-block:: console

  sudo dpkg -i ~/Downloads/JLink_Linux_V656a_x86_64.deb
  sudo apt install -f

.. note:
  
  The name of the exact file may be slightly different depending on what version you downloaded so copying and pasting the above may not work.

After installing the J-Link software package please restart your terminal emulator and try to tab complete and open the program JLinkExe. If you are successful you will be greeted by the following:

.. code-block:: console

  SEGGER J-Link Commander V6.56 (Compiled Nov 22 2019 17:14:15)
  DLL version V6.56, compiled Nov 22 2019 17:14:02
  Followed by a can not connect error if the J-Link is not connected. Close JLinkExe with exit.

JLink Ozone debugger
********************

`Ozone <https://www.segger.com/products/development-tools/ozone-j-link-debugger/>`_ is a free cross-platform 
debugger and performance analyzer for J-Link and J-Trace. It's advantages are that it is completely 
standalone and that it is very quick to get it set up with your project. This is a very capable debugger
with far too many features to list here, so check out the website for more information on what Ozone 
can do.

It is reccommended that you use the `JLink EDU Mini`_ or JLink EDU debugger probes with Ozone. 

You can download the appropriate version of Ozone for your OS from the `website <https://www.segger.com/downloads/jlink/#Ozone>`_.

.. _JLink EDU MINI: https://shop-us.segger.com/J_Link_EDU_mini_p/8.08.91.htm

Setup
=====

Once you have downloaded and installed Ozone onto your computer, and have installed the `J-Link Software Pack <installing_jlink_software_>`_,
you are ready to set up the debugger with your project. 

1. Launch Ozone
2. Run the `New Project Wizard` by going to `File->New->New Project Wizard`
3. Under `Device`, select the target STM32 chip that you are debugging and click next. 
4. Change the Target and Host Interface settings to match your debugger probe and click next.
5. Select the compiled executable of your program located in the build folder of your project. To get all of the 
   debug symbols, you will want to use the `*.elf` output file. Click finish to complete setup.

That's it! You are now ready to debug your application. 

Basic Debugging
===============

This guide will go over some basic features to get you started debugging, and you can check out the `Ozone User's Manual`
for a complete reference on how to use the program. 

.. _Ozone User's Manual: https://www.segger.com/downloads/jlink/UM08025_Ozone.pdf

Connect to Target and Download Application
------------------------------------------

- Make sure that your board is power up, and that you have connected your debug probe
   to the board and your host computer. 
- Click on `Debug->Start Debug Session->Download and Reset Program` to start the debugging session and 
   set a breakpoint at the beginning of the application's `main()` function.

Breakpoints and Stepping Through the Code
-----------------------------------------

Watching Variables
------------------


VSCode / VSCodium Setup
***********************


Cortex-Debug
============

The Cortex-Debug extension is the glue that pulls in all of the capabilites of GDB into VSCode.
You will need to enable the extension from the extension marketplace. 

In your VSCode :file:`.vscode/settings.json` file, add the following line to let Cortex-Debug know
where your JLink GDB server executable lives:

.. tabs::
   .. group-tab:: Ubuntu
        .. code-block:: json

            "cortex-debug.JLinkGDBServerPath": "/opt/SEGGER/JLink/JLinkGDBServerCLExe"

You then need to add a debug configuration to your :file:`.vscode/launch.json`. For the reccommended setup,
use the below configuration, editing the `device`, `interface`, and `executable` parameters to match your project.

.. code-block:: json

{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug",
      "cwd": "${workspaceRoot}",
      "executable": "./build/debug/myApp.elf",
      "request": "launch",
      "type": "cortex-debug",
      "servertype": "jlink",
      "interface": "jtag",
      "device": "STM32F769NI",
      "runToMain": true,
      "preRestartCommands": [
        "load",
        "enable breakpoint",
        "monitor reset"
      ]
    }
  ]
}