MAKE_FLAGS = --no-print-directory
TESTS = *

# Similar to the above build target command, but for firmware.  This is used
# because CMake can only handle one toolchain at a time, so we build the MBED-
# targeted code separately.
define cmake_build_target
	mkdir -p build
	# CMAKE_BUILD_TYPE Debug or Release
	cd build && cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev --target $1 $2 .. && make $1 $(MAKE_FLAGS) -j
endef

# TODO: what?
all:
	$(call cmake_build_target, all)

# Targets to be uploaded to robot
FIRMWARE_UPLOADS = control kicker fpga base
$(FIRMWARE_UPLOADS):
	$(call cmake_build_target, $(@F))
$(FIRMWARE_UPLOADS:%=%-prog):
	$(call cmake_build_target, $(@F))


# Run both C++ and python unit tests
tests: test-firmware
test-firmware:
	$(call cmake_build_target, test-firmware)
	run/test-firmware --gtest_filter=$(TESTS)

clean:
	cd build && ninja clean || true
	rm -rf build


fpga-test:
	$(call cmake_build_target, fpga_iverilog)
fpga-test-strict:
	$(call cmake_build_target, fpga_iverilog_strict)

FIRMWR_TESTS := $(patsubst src/hw-test/test-%.cpp,%,$(wildcard src/hw-test/test-*.cpp))
# robot-test-<test_unit>{-prog}
# defines the targets described at the line above - test units defined in FIRMWR_TESTS
$(FIRMWR_TESTS:%=robot-test-%):
	$(call cmake_build_target, robot-test, -DHW_TEST_UNIT:STRING=$(@F:robot-test-%=%))
$(FIRMWR_TESTS:%=robot-test-%-prog):
	$(call cmake_build_target, robot-test-prog, -DHW_TEST_UNIT:STRING=$(@F:robot-test-%-prog=%))

GDB_PORT ?= 3333
.INTERMEDIATE: build/control-gdb.pid
build/control-gdb.pid:
# this will cache sudo use without a password in the environment
# so we won't enter the gdb server and skip past the password prompt.
	@sudo echo "starting pyocd-gdbserver, logging to build/control-gdb.log"
# now we can refresh the sudo timeout and start up the gdb server
	sudo -v && { sudo pyocd-gdbserver --allow-remote --port $(GDB_PORT) --reset-break \
	--target lpc1768 -S -G > build/control-gdb.log 2>&1 & sudo echo $$! > $@; }

GDB_NO_CONN ?= 0
control-gdb: control build/control-gdb.pid
# use pyocd-gdbserver, and explicitly pass it the type of target we want to connect with,
# making sure that we enable semihosting and use gdb syscalls for the file io
	@trap 'sudo pkill -9 -P `cat build/control-gdb.pid`; exit' TERM INT EXIT && \
	if [ $(GDB_NO_CONN) -eq 0 ]; then \
		arm-none-eabi-gdb build/src/control/control_elf \
		  -ex "target remote localhost:$(GDB_PORT)" \
		  -ex "load" \
		  -ex "tbreak main" \
		  -ex "continue"; \
	else \
		while true; do sleep 10; done; \
	fi


# Build all of the firmware for a robot, and/or move all of the binaries over to the mbed
firmware: control kicker fpga base
firmware-prog: control-prog kicker-prog fpga-prog


STYLIZE_DIFFBASE ?= master
STYLE_EXCLUDE_DIRS=build \
				   external \
				   run \
				   lib/drivers/decawave/decadriver
# automatically format code according to our style config defined in .clang-format
pretty:
	@stylize --diffbase=$(STYLIZE_DIFFBASE) --clang_style=file --yapf_style=.style.yapf --exclude_dirs $(STYLE_EXCLUDE_DIRS)
# check if everything in our codebase is in accordance with the style config defined in .clang-format
# a nonzero exit code indicates that there's a formatting error somewhere
checkstyle:
	@printf "Run this command to reformat code if needed:\n\ngit apply <(curl -L $${LINK_PREFIX:-file://}clean.patch)\n\n"
	@stylize --diffbase=$(STYLIZE_DIFFBASE) --clang_style=file --yapf_style=.style.yapf --exclude_dirs $(STYLE_EXCLUDE_DIRS) --check --output_patch_file="$${CIRCLE_ARTIFACTS:-.}/clean.patch"


apidocs:
	doxygen doc/Doxyfile
	cp doc/doxygen.css api_docs/html/
	@echo "\n=> Open up 'api_docs/html/index.html' in a browser to view a local copy of the documentation"
