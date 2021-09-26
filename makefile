.PHONY : all flash kicker kicker-test control docs clean $(ROBOT_TESTS:%=%)

all: control flash

flash:
	./util/flash-mtrain

flash-test:
	./util/flash-mtrain-test

kicker:
	cd kicker && \
mkdir -p build && cd build && \
cmake .. && make -j$(nproc) && cd .. && \
python3 convert.py build/bin/kicker.nib build/bin/kicker_bin.h KICKER_BYTES

kicker-test:
	cd kicker && \
mkdir -p build && cd build && \
cmake .. && make -j$(nproc) kicker-test && cd .. && \
python3 convert.py build/bin/kicker-test.nib build/bin/kicker_bin.h KICKER_BYTES

ROBOT_TESTS = rtos icm-42605-angle icm-20498-rate icm-20498-angle radio-test

control:
	cd control && \
mkdir -p build && cd build && \
cmake .. && make -j$(nproc)

$(ROBOT_TESTS:%=%): kicker-test
	cd control && \
mkdir -p build && cd build && \
cmake .. && make -j$(nproc) $(@F)
	make flash-test

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"

clean:
	cd control && rm -rf build
	cd control/mtrain && make clean
	cd fpga && rm -rf build
	cd kicker && rm -rf build
	rm -rf generated-docs
