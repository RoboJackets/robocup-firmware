# MIT License

# Copyright (c) 2012-2017 Konstantin Oblaukhov

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set( CMAKE_CXX_STANDARD 17 CACHE STRING "")
set( CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" CACHE PATH "")
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib" CACHE PATH "")
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib" CACHE PATH "")

set(ARM_PREFIX                    arm-none-eabi-gcc)
find_program(CMAKE_C_COMPILER     ${ARM_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER   ${ARM_PREFIX}-g++)

set(C_MACHINE_OPTIONS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard" CACHE STRING "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_MACHINE_OPTIONS}" CACHE STRING "")
# TODO Check if we should be using no-rtti
# cpp 17 deprecated register storage specifier and cmsis uses it everywhere
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_MACHINE_OPTIONS} -Wno-register -fno-exceptions -fno-rtti" CACHE STRING "")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${C_MACHINE_OPTIONS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs --specs=nano.specs -T '${CMAKE_CURRENT_LIST_DIR}/../BSP/flash.ld' -u _printf_float" CACHE STRING "")

# TODO Trim the Duse usb and stm stuff
add_definitions(-Wall -DSTM32F769xx -DUSE_USB_HS)
set(CHIP STM32F769NI CACHE STRING "Full STM32 Chip Model")
set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "")

get_filename_component(STM32_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
list(APPEND CMAKE_MODULE_PATH ${STM32_CMAKE_DIR})

if(NOT STM32_TOOLCHAIN_PATH)
     set(STM32_TOOLCHAIN_PATH "/usr/lib/arm-none-eabi")
else()
     file(TO_CMAKE_PATH "${STM32_TOOLCHAIN_PATH}" STM32_TOOLCHAIN_PATH)
endif()

set(STM32_TARGET_TRIPLET "arm-none-eabi")

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_SYSROOT  "${STM32_TOOLCHAIN_PATH}/${STM32_TARGET_TRIPLET}")
set(TOOLCHAIN_BIN_PATH "${STM32_TOOLCHAIN_PATH}/bin")
set(TOOLCHAIN_INC_PATH "${STM32_TOOLCHAIN_PATH}/${STM32_TARGET_TRIPLET}/include")
set(TOOLCHAIN_LIB_PATH "${STM32_TOOLCHAIN_PATH}/${STM32_TARGET_TRIPLET}/lib")

message(STATUS "TOOLCHAIN_BIN_PATH: ${TOOLCHAIN_BIN_PATH}")

find_program(CMAKE_OBJCOPY NAMES ${STM32_TARGET_TRIPLET}-objcopy PATHS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_OBJDUMP NAMES ${STM32_TARGET_TRIPLET}-objdump PATHS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_SIZE NAMES ${STM32_TARGET_TRIPLET}-size PATHS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_DEBUGGER NAMES ${STM32_TARGET_TRIPLET}-gdb PATHS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_CPPFILT NAMES ${STM32_TARGET_TRIPLET}-c++filt PATHS ${TOOLCHAIN_BIN_PATH})

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
find_program(CMAKE_C_COMPILER NAMES ${STM32_TARGET_TRIPLET}-gcc PATHS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_CXX_COMPILER NAMES ${STM32_TARGET_TRIPLET}-g++ PATHS ${TOOLCHAIN_BIN_PATH})
find_program(CMAKE_ASM_COMPILER NAMES ${STM32_TARGET_TRIPLET}-gcc PATHS ${TOOLCHAIN_BIN_PATH})
