cmake_minimum_required(VERSION 3.4)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )
include(attiny_toolchain.cmake)

# compile executable for kicker firmware
add_executable(kicker.elf
    main.c
)

target_include_directories(kicker.elf PUBLIC
    ${PROJECT_SOURCE_DIR}/../robot/lib/Inc/drivers/Internal
)

set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})
# custom target to convert kicker.elf to the kickerFW binary file and place it in the 'run' directory
add_custom_target(kicker ALL
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)

 # add command to copy the kicker firmware onto the mbed
 add_custom_target(kicker-upload
     COMMAND ${MBED_COPY_SCRIPT} kicker.nib
     WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
     DEPENDS kicker
     COMMENT "Copying the kicker board's binary over to the mbed"
 )

# F_CPU specifies the frequency (in Hz) of the cpu clock so that the delay functions work appropriately
# the ATTiny13 has an internal 9.6MHz clock with the prescaler, don't scale back
# change above to match ATTINY84A =====================================================================
# to keep timing for neopixel
# The -O2 turns on optimizations, which are required to use the delay_*() functions
# list(APPEND CMAKE_C_FLAGS "-std=c11 -O2 -mmcu=${AVR_TARGET_ARCH} -DF_CPU=8000000")
list(APPEND CMAKE_C_FLAGS "-std=c11 -O2 -mmcu=${AVR_TARGET_ARCH} -DF_CPU=8000000")
