<<<<<<< HEAD
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
=======
.PHONY : all flash kicker kicker-test configure robot docs $(ROBOT_TESTS:%=test-%-upload)

all: robot flash

flash:
	./util/flash-mtrain

flash-test:
	./util/flash-mtrain-test

kicker:
	cd kicker && \
mkdir -p build && cd build && \
cmake -DCMAKE_TOOLCHAIN_FILE=../atmega_toolchain.cmake .. && make && cd .. && \
python3 convert.py build/bin/kicker.nib ../robot/lib/Inc/device-bins/kicker_bin.h KICKER_BYTES

kicker-test:
	cd kicker && \
mkdir -p build && cd build && \
cmake -DCMAKE_TOOLCHAIN_FILE=../atmega_toolchain.cmake .. && make kicker-test && cd .. && \
python3 convert.py build/bin/kicker-test.nib ../robot/lib/Inc/device-bins/kicker_bin.h KICKER_BYTES

# Define BUILDTYPE as Release if not already set for this target and subtargets
robot/build/conaninfo.txt : BUILDTYPE ?= "Release"
robot/build/conaninfo.txt : robot/conanfile.py
	cd robot && conan install . -if build -pr armv7hf -r robojackets -s build_type=$(BUILDTYPE) --build missing

configure : robot/build/conaninfo.txt
	cd robot && conan build . -bf build -c

ROBOT_TESTS = rtos icm-42605-angle

robot: robot/build/conaninfo.txt
	cd robot && conan build . -bf build

$(ROBOT_TESTS:%=%): configure
	cd robot/build; make $(@F)
	make flash-test
>>>>>>> a441d5820ed2613e795a1a3dc001243fa67a98b5

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
