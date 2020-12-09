.PHONY : all control kicker fpga docs clean

CURRENT_DIR=$(shell pwd)

CONVERT_SCRIPT="$(CURRENT_DIR)/convert.py"
FLASH_COPY_SCRIPT="$(CURRENT_DIR)/util/flash-mtrain.py"

all: control

control-upload: control

# control: fpga kicker
control: kicker
	cd control && \
mkdir -p build && cd build && \
cmake -DFLASH_COPY_SCRIPT=${FLASH_COPY_SCRIPT} .. && make -j$(nproc)

# fpga:
# 	cd fpga && \
# mkdir -p build && cd build && \
# cmake  .. && make -j$(nproc)

kicker:
	cd kicker && \
mkdir -p build && cd build && \
cmake -DCONVERT_SCRIPT=${CONVERT_SCRIPT} .. && make -j$(nproc)

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"

clean:
	cd control && rm -rf build
	cd control/mtrain && make clean
	cd fpga && rm -rf build
	cd kicker && rm -rf build
