cmake_minimum_required(VERSION 3.14.7)

# set the AVR toolchain
set( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/atmega_toolchain.cmake )

project(kicker LANGUAGES C)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )
include(atmega_toolchain.cmake)

# compile executable for kicker firmware
add_executable(kicker.elf
    main.c
)

add_executable(kicker-test.elf
    test.c
)

target_include_directories(kicker.elf PUBLIC
    include
)

set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})
# custom target to convert kicker.elf to the kicker.nib binary file and place it in the 'run' directory
add_custom_target(kicker ALL
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)

# custom target to convert kicker-test.elf to the kicker-test.nib binary file and place it in the 'run' directory
add_custom_target(kicker-test ALL
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker-test.elf kicker-test.nib
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker-test.elf
    COMMENT "objcopying to convert kicker-test.elf to kicker-test.nib"
)

# F_CPU specifies the frequency (in Hz) of the cpu clock so that the delay functions work appropriately
# the ATTiny13 has an internal 9.6MHz clock with the prescaler, don't scale back
# change above to match ATTINY84A =====================================================================
# to keep timing for neopixel
# The -O2 turns on optimizations, which are required to use the delay_*() functions
list(APPEND CMAKE_C_FLAGS "-std=c11 -O2 -mmcu=${AVR_TARGET_ARCH} -DF_CPU=8000000")
