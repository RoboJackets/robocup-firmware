.PHONY : all flash kicker kicker-test control tests docs clean $(ROBOT_TESTS)

ROBOT_TESTS = rtos icm-42605-angle radio-test blink

ROBOT_TESTS = rtos icm-42605-angle radio-test

all: control flash

tests: control-tests kicker-test 

all-kicker: kicker kicker-test

flash:
	./util/flash-mtrain

flash-test:
	./util/flash-test $(TEST)

kicker:
	cd kicker && \
mkdir -p build && cd build && \

cmake .. && make -j$(nproc) kicker && cd .. && \

python3 convert.py build/bin/kicker.nib build/bin/kicker_bin.h KICKER_BYTES

kicker-test:
	cd kicker && \
mkdir -p build && cd build && \
cmake .. && make -j kicker-test && cd .. && \
python3 convert.py build/bin/kicker-test.nib build/bin/kicker_bin.h KICKER_BYTES

control:
	@if [[ ! -f kicker/build/bin/kicker_bin.h ]]; then echo "=> Please build either 'kicker' or 'kicker-test' prior to building control"; exit 1; else true; fi
	cd control && \
mkdir -p build && cd build && \
cmake .. && make -j control

$(ROBOT_TESTS): kicker-test
	cd control && \
mkdir -p build && cd build && \
cmake .. && make -j$(nproc) control

control-tests:
	cd control && \
mkdir -p build && cd build && \
cmake .. && make -j$(nproc) $(ROBOT_TESTS)

$(ROBOT_TESTS): kicker-test
	cd control && \
mkdir -p build && cd build && \
cmake .. && make -j $(@F)
	make flash-test TEST=$(@F)

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"

clean:
	cd control && rm -rf build
	cd control/mtrain && make clean
	cd fpga && rm -rf build
	cd kicker && rm -rf build
	rm -rf generated-docs

CLANG_FORMAT_BINARY=clang-format-10

# run locally before committing
pretty-lines:
	@git diff $(DIFFBASE) -U0 --no-color | python3 util/style/clang-format-diff.py -binary $(CLANG_FORMAT_BINARY) -i -p1
	@git diff -U0 --no-color $(DIFFBASE) | black . --exclude "control/mtrain/"
