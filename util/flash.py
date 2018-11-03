import subprocess
import sys

# output = subprocess.check_output(('JLinkExe'), stdin='q')
# print(output)

# testString = """
# device STM32F769NI
# JTAGConf -1 -1
# speed 4000
# loadbin build/tests/blink.bin 0x08000000
# q
# """

if len(sys.argv) < 2:
    print("No file provided")
    exit()

fileName = sys.argv[1]
flashAddress = "0x08000000"

testString = (
    "device STM32F769NI\n"
    "JTAGConf -1 -1\n"
    "speed 4000\n"
    "loadbin " + fileName + " " + flashAddress + "\n"
    "q\n"
)

p = subprocess.Popen('JLinkExe', stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
# out1 = p.communicate(input=b'device STM32F769NI\nJTAGConf -1 -1\nspeed 4000\nloadbin build/tests/blink.bin 0x08000000\nq\n')[0]
pOut = p.communicate(input=testString.encode())[0].decode()
if 'Connecting to J-Link via USB...FAILED' in pOut:
    print('Connection to J-Link failed')
if 'Cannot connect to target' in pOut:
    print('Connection to MCU failed')

# out1 = p.communicate(input=b'usb\n')[0]
# p.communicate(input=b'q\n')
# print(pOut)