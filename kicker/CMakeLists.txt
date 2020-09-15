cmake_minimum_required(VERSION 3.14.7)

# set the AVR toolchain
set( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/attiny_toolchain.cmake )

project(kicker LANGUAGES C)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )

# compile executable for kicker firmware
add_executable(kicker.elf
    src/main.c
    src/HAL_attiny167.c
)

target_include_directories(kicker.elf PUBLIC
    include
)

# custom target to convert kicker.elf to the kickerFW binary file
add_custom_target(kicker ALL
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    COMMAND echo "STARTING THE PAIN TRAIN"
    COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    COMMAND python3 ${CONVERT_SCRIPT} kicker.nib kicker_bin.h KICKER_BYTES
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)
