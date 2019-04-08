cmake_minimum_required(VERSION 3.1)

project( mTrain
         VERSION ${mTrain_VERSION} # set by Conan
         LANGUAGES     C  CXX
)

# TODO: don't leave these here
add_definitions(-Wall)
set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_STANDARD_REQUIRED  ON )

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -T '${CMAKE_SOURCE_DIR}/BSP/flash.ld'")

set(FLASH_COPY_SCRIPT python3 ${PROJECT_SOURCE_DIR}/util/flash.py)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib" )

# TODO: remove need for USE_USB_HS def
# Definitions for HAL
add_definitions(-DSTM32F769xx -DUSE_USB_HS)
add_subdirectory(external)

add_subdirectory(API/c)
add_subdirectory(API/cpp)
add_subdirectory(BSP)

if( IS_DIRECTORY  "${CMAKE_CURRENT_LIST_DIR}/tests" )
    add_subdirectory(tests/c)
    add_subdirectory(tests/cpp)
endif()