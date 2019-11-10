.PHONY : all kicker configure robot control-upload $(ROBOT_TESTS:%=test-%-upload)

all: kicker robot

kicker:
	cd kicker && \
mkdir -p build && cd build && \
cmake -DCMAKE_TOOLCHAIN_FILE=../attiny_toolchain.cmake .. && make


robot/build/conaninfo.txt : robot/conanfile.py
	cd robot && conan install . -if build -pr armv7hf --build missing
configure : robot/build/conaninfo.txt
	cd robot && conan build . -bf build -c


ROBOT_TESTS = test

robot : robot/build/conaninfo.txt
	cd robot && conan build . -bf build
control-upload: configure
	cd robot/build; make control-upload
$(ROBOT_TESTS:%=test-%-upload): configure
	cd robot/build; make $(@F)

clean:
	rm -rf kicker/build
	rm -rf robot/build
