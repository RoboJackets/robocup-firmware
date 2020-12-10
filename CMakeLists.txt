cmake_minimum_required(VERSION 3.4)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/stm32-gcc-arm-none-eabi.cmake)

project(
    mTrain
    LANGUAGES C CXX ASM
)
set(FLASH_COPY_SCRIPT python3 ${CMAKE_CURRENT_LIST_DIR}/util/flash.py)

set( CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_STANDARD_REQUIRED  ON )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib" )

set(C_MACHINE_OPTIONS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_MACHINE_OPTIONS}")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_MACHINE_OPTIONS} '-fno-exceptions -fno-rtti'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_MACHINE_OPTIONS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${C_MACHINE_OPTIONS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs --specs=nano.specs -T '${CMAKE_CURRENT_LIST_DIR}/BSP/flash.ld'")

# TODO: don't leave these here
add_definitions(-Wall)
set(CMAKE_BUILD_TYPE Debug)

# TODO: remove need for USE_USB_HS def
# Definitions for HAL
add_definitions(-DSTM32F769xx -DUSE_USB_HS)
set(CHIP STM32F769NI CACHE STRING "Full STM32 Chip Model")

add_subdirectory(external)

add_subdirectory(API/c)
add_subdirectory(API/cpp)
add_subdirectory(BSP)

add_library(mtrain INTERFACE)

target_link_libraries(mtrain INTERFACE
    STM32F7xx
    STM32F7_API_C
    STM32F7_API_CPP
)

if( IS_DIRECTORY  "${CMAKE_CURRENT_LIST_DIR}/tests" )
    add_subdirectory(tests/c)
    add_subdirectory(tests/cpp)
endif()
