cmake_minimum_required(VERSION 3.0.0)
project("GT RoboJackets RoboCup Firmware")


# include cmake files in the 'cmake folder'
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# put executables in the 'run' folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/run)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/run)

# Google test
enable_testing()
include(SetupGTest)

# C++ version
if ("${CMAKE_VERSION}" LESS "3.1")
    # This is to support the CMake version backported to Ubuntu 14.04
    message("CMAKE_VERSION < 3.1, specifying c++14 flag.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
    # This is the more modern way of setting our c++ version
    message("CMAKE_VERSION >= 3.1, setting CMAKE_CXX_STANDARD to 14.")
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON) # Don't fall back to older versions
endif()

# Because we use ninja, we have to explicitly turn on color output for the compiler
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics -Werror=return-stack-address")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-local-addr")

endif()

# Build in debug mode
# TODO: make this easier to specify when running `make` on the command-line
# try using cmake's debug flag support? 'set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Turn some compiler warnings into errors
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -Werror=delete-non-virtual-dtor -Werror=uninitialized")

# Google Protobuf
find_package(Protobuf REQUIRED)
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIR})

# Eigen - used for linear algebra
find_package(Eigen3 REQUIRED)

# libusb
find_package(libusb-1.0 REQUIRED)

# SDL
find_package(SDL REQUIRED)



# run all the other CMakeLists files
# add_subdirectory(common)
# add_subdirectory(firmware)

# This sets up MBED all MBED libraries, then sets up the robot and base station
# firmware, which depend on the MBED libs
include(RemoveFlags)

# include Eigen3 for linear algebra
include_directories(${EIGEN_INCLUDE_DIR})

# remove the color flag since it will get set on anything with clang assert
# the default compiler (mostly apple computers). The arm compilers know
# nothing about this flag however.
remove_cxx_flag("-fcolor-diagnostics")
remove_cxx_flag("-Werror=return-stack-address")

# sets many variables and paths related to building for the MBED
include(${CMAKE_CURRENT_LIST_DIR}/lib/mbed/arm_mbed.cmake)

# set the path to a script for moving files to the mbed's USB storage
# passes the MBED_INDEX environment variable as a parameter if it is set
if(DEFINED ENV{MBED_INDEX})
    set(MBED_COPY_SCRIPT sudo ${PROJECT_SOURCE_DIR}/util/mbed/mbed-copy.py --mbed_index=$ENV{MBED_INDEX})
else()
    set(MBED_COPY_SCRIPT sudo ${PROJECT_SOURCE_DIR}/util/mbed/mbed-copy.py)
endif()

# Note: the arm_mbed.cmake script exports flags that need to be set in each CMake
# file used to build MBED code like so:
# set(CMAKE_CXX_FLAGS ${MBED_CMAKE_CXX_FLAGS})
# set(CMAKE_C_FLAGS ${MBED_CMAKE_C_FLAGS})
# set(CMAKE_EXE_LINKER_FLAGS ${MBED_CMAKE_EXE_LINKER_FLAGS})

# enable C++ exceptions
set(MBED_CMAKE_CXX_FLAGS "${MBED_CMAKE_CXX_FLAGS} -fexceptions")
# the mbed libraries are compiled with -fno-rtti, so we need to as well
set(MBED_CMAKE_CXX_FLAGS "${MBED_CMAKE_CXX_FLAGS} -fno-rtti")
# enable C++14
set(MBED_CMAKE_CXX_FLAGS "${MBED_CMAKE_CXX_FLAGS} -std=c++14")
# Enable logging for the common2015 library
set(MBED_CMAKE_CXX_FLAGS "${MBED_CMAKE_CXX_FLAGS} -DRJ_LOGGING_EN")
# ignore formatting flags for printf
set(MBED_CMAKE_CXX_FLAGS "${MBED_CMAKE_CXX_FLAGS} -Wno-format -Wno-switch -Wno-unused-parameter -Wno-missing-field-initializers")
# generate debugging information for gdb
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# suppress -rdynamic flag
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Add a test runner target "test-firmware"
file(GLOB FIRMWARE_TEST_SRC "lib/test/*.cpp")
add_executable(test-firmware ${FIRMWARE_TEST_SRC})
add_dependencies(test-firmware googletest)
target_link_libraries(test-firmware ${GTEST_LIBRARIES})

# Don't build the tests by default
set_target_properties(test-firmware PROPERTIES EXCLUDE_FROM_ALL TRUE)

# build robot and base station firmware and the library that they depend on
add_subdirectory(lib/mbed)

add_subdirectory(lib)
add_subdirectory(src)