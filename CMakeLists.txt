cmake_minimum_required(VERSION 3.1)
project(Unnamed LANGUAGES C)

if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "ARM")
    message(FATAL_ERROR "Cannot be compiled for architectures other than ARM \
    (Current=${CMAKE_SYSTEM_PROCESSOR})")
endif()

# TODO: probably should not be here
# Show all warnings
add_definitions(-Wall)

# TODO: don't leave this here
set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_CXX_STANDARD 14)

# Flags for machine options for STM32F769xx
set (C_MACHINE_OPTIONS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs")

set (CMAKE_C_FLAGS "${C_MACHINE_OPTIONS} -std=gnu99" CACHE INTERNAL "c compiler flags")
set (CMAKE_CXX_FLAGS "${C_MACHINE_OPTIONS}" CACHE INTERNAL "c compiler flags")
set (CMAKE_ASM_FLAGS "${C_MACHINE_OPTIONS} -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")

set (CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections ${C_MACHINE_OPTIONS} --specs=nosys.specs"
    CACHE INTERNAL "executable linker flags")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -T '${CMAKE_SOURCE_DIR}/BSP/flash.ld'")


set(FLASH_COPY_SCRIPT python3 ${PROJECT_SOURCE_DIR}/util/flash.py)

# TODO: remove need for USE_USB_HS def
# Definitions for HAL
add_definitions(-DSTM32F769xx -DUSE_USB_HS)
add_subdirectory(external)

add_subdirectory(API/c)
add_subdirectory(API/cpp)
add_subdirectory(BSP)

add_subdirectory(tests/c)
add_subdirectory(tests/cpp)
