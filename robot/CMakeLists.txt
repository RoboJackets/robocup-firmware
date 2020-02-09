cmake_minimum_required(VERSION 3.4)

project(RoboCupFirmware
    LANGUAGES C CXX
)

set(CMAKE_NO_SYSTEM_FROM_IMPORTED True)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )

add_definitions(-Wall)

# TODO: remove
add_definitions(-Wno-register)

# C++ version
set(CMAKE_CXX_STANDARD 17)
# Don't fall back to older versions
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

add_subdirectory(control)

add_subdirectory(hw-test)
