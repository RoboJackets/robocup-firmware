all : build/conaninfo.txt
	conan build . -bf build

C_FIRMWARE_TESTS = blink blink_interrupt gpio flash usb_serial spi us_delay rtos adc
CPP_FIRMWARE_TESTS = blink gpio spi usb_serial i2c i2c_bus_recovery

$(C_FIRMWARE_TESTS:%=upload-%-c): configure
	cd build; make $(@F)
$(CPP_FIRMWARE_TESTS:%=upload-%): configure
	cd build; make $(@F)

build/conaninfo.txt : conanfile.py
	conan install . -if build -pr armv7hf --build missing

.PHONY : configure
configure : build/conaninfo.txt
	conan build . -bf build -c

clean:
	rm -rf build

docs:
	cd doc && doxygen Doxyfile
	@echo "\n=> Open up 'generated-docs/index.html' in a browser to view a local copy of the documentation"
