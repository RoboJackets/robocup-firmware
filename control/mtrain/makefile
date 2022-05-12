.PHONY: all mtrain flash tests docs clean $(CPP_FIRMWARE_TESTS)

CPP_FIRMWARE_TESTS = adc leds gpio spi usb-serial i2c

all : mtrain

mtrain:
	mkdir -p build && cd build && \
cmake .. && make -j

$(CPP_FIRMWARE_TESTS):
	mkdir -p build && cd build && \
cmake .. && make -j $(@F) && \
python3 ../util/flash.py bin/$(@F).bin

tests: mtrain

clean:
	rm -rf build
	rm -rf generated-docs

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"
