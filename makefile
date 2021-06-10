.PHONY: all mtrain tests docs clean

C_FIRMWARE_TESTS = blink blink_interrupt gpio flash usb_serial spi us_delay rtos adc
CPP_FIRMWARE_TESTS = blink gpio spi usb_serial i2c i2c_bus_recovery

all : mtrain tests

mtrain:
	mkdir -p build && cd build && \
cmake .. && make -j$(nproc)

$(C_FIRMWARE_TESTS:%=upload-%-c): tests
	cd build; make $(@F) -j$(nproc)
$(CPP_FIRMWARE_TESTS:%=upload-%): tests
	cd build; make $(@F) -j$(nproc)

clean:
	rm -rf build

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"
