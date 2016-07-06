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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

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

# Several things depend on the headers in the 'common' directory
include_directories("${PROJECT_SOURCE_DIR}/common") # for headers in common/
include_directories("${PROJECT_BINARY_DIR}/common") # for generated protobuf headers

# run all the other CMakeLists files
add_subdirectory(common)
add_subdirectory(firmware)
