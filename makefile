.PHONY : all control kicker fpga

CURRENT_DIR=$(shell pwd)

CONVERT_SCRIPT="$(CURRENT_DIR)/convert.py"
FLASH_COPY_SCRIPT="$(CURRENT_DIR)/util/flash-mtrain.py"

all: control

control-upload: control

# control: fpga kicker
control: kicker
	cd control && \
mkdir -p build && cd build && \
cmake -DFLASH_COPY_SCRIPT=${FLASH_COPY_SCRIPT} .. && make

# fpga:
# 	cd fpga && \
# mkdir -p build && cd build && \
# cmake  .. && make

kicker:
	cd kicker && \
mkdir -p build && cd build && \
cmake -DCONVERT_SCRIPT=${CONVERT_SCRIPT} .. && make

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"

clean:
	cd control && rm -rf build
	cd fpga && rm -rf build
	cd kicker && rm -rf build
