#!/usr/bin/env python3

#
# Converts the bin/nib file into a C header
#
# Both the fpga and kicker bins must be converted into a header
# so we can dynamically program them on robot startup
# This is the easy fix until a file system is built
#
# Example usage:
# python3 convert.py kicker/build/bin/kicker.nib robot/lib/Inc/device-bins/kicker_bin.h KICKER_BYTES
#
# FPGA should be very similar
#

import sys

if len(sys.argv) != 4:
	print("Usage: ./convert.py <input.bin> <output.h> <buf_name>")
	exit(-1)

data = None
with open(sys.argv[1], 'rb') as file:
	data = file.read()

if data is None:
	print("Failed to open file:" + str(sys.argv[1]))
	exit(-1)

num_bytes = len(data)
hdr_sym = sys.argv[2].upper().replace(".", "_").replace("-", "_")

print(str(sys.argv[1]) + " has " + str(num_bytes) + " bytes of data.")

print("Building header file...")

file_string = ""
file_string += "#ifndef " + hdr_sym + "\n"
file_string += "#define " + hdr_sym + "\n"
file_string += "\n"
file_string += "static const uint32_t " + str(sys.argv[3]).upper() + "_LEN = " + str(num_bytes) + ";\n"
file_string += "static const uint8_t " + str(sys.argv[3]) + "[" + str(num_bytes) + "] = {"

for i in range(num_bytes):
	file_string += hex(data[i])
	if i != num_bytes - 1:
		file_string += ", "

file_string += "};\n"
file_string += "\n"
file_string += "#endif //" + hdr_sym

print("Done.")

print("Opening and writing " + sys.argv[2])

f = open(sys.argv[2], 'w+')
f.write(file_string)

print('Done.')
