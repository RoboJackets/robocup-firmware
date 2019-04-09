
all : build/conaninfo.txt
	conan build . -bf build

C_FIRMWARE_TESTS = blink gpio flash usb_serial spi
CPP_FIRMWARE_TESTS = blink gpio spi

$(C_FIRMWARE_TESTS:%=upload-%-c): configure
	cd build; make $(@F)
$(CPP_FIRMWARE_TESTS:%=upload-%): configure
	cd build; make $(@F)

clean:
	rm -rf build


build/conaninfo.txt : conanfile.py
	conan install . -if build -pr armv7hf

.PHONY : configure
configure : build/conaninfo.txt
	conan build . -bf build -c