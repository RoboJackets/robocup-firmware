#!/usr/bin/env python3

import subprocess
import sys
import shlex

if len(sys.argv) < 2:
    print("No file provided")
    exit()

fileName = sys.argv[1]
flashAddress = "0x08000000"

testString = (
    "r\n"
    "loadbin \"" + fileName + "\" " + flashAddress + "\n"
    "r\n"
    "g\n"
    "q\n"
)

p = subprocess.Popen(shlex.split("JLinkExe -device STM32F769NI -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1"), stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
pOut = p.communicate(input=testString.encode())[0].decode()
if 'Connecting to J-Link via USB...FAILED' in pOut or 'Cannot connect to J-Link via USB' in pOut:
    sys.exit('Connection to J-Link failed')
if 'Cannot connect to target' in pOut:
    sys.exit('Connection to MCU failed')
# TODO: more checks if something failed
