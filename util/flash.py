import subprocess
import sys

if len(sys.argv) < 2:
    print("No file provided")
    exit()

fileName = sys.argv[1]
flashAddress = "0x08000000"

testString = (
    "device STM32F769NI\n"
    "JTAGConf -1 -1\n"
    "speed 4000\n"
    "r\n"
    "loadbin " + fileName + " " + flashAddress + "\n"
    "r\n"
    "g\n"
    "q\n"
)

p = subprocess.Popen('JLinkExe', stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
pOut = p.communicate(input=testString.encode())[0].decode()
if 'Connecting to J-Link via USB...FAILED' in pOut or 'Cannot connect to J-Link via USB' in pOut:
    sys.exit('Connection to J-Link failed')
if 'Cannot connect to target' in pOut:
    sys.exit('Connection to MCU failed')
# TODO: more checks if something failed