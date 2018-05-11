
define cmake_build_target
	mkdir -p build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=arm_toolchain.cmake .. && make
endef

all:
	$(call cmake_build_target, all)

clean:
	rm -rf build
