cmake_minimum_required(VERSION 3.4)

project(kicker
    LANGUAGES C
    )

# set the AVR toolchain
set( CMAKE_TOOLCHAIN_FILE attiny_toolchain.cmake )
include(attiny_toolchain.cmake)
set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )

# compile executable for kicker firmware
add_executable(kicker.elf
    src/main.c
    src/HAL_attiny167.c
)

target_include_directories(kicker.elf PUBLIC
    include
)

# NOTE control should be dependent on this target and I think the includes should transfer through to the control target

# TODO FIX THE NIB OUTPUT PATH
# custom target to convert kicker.elf to the kickerFW binary file
add_custom_target(kicker
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    COMMAND mkdir -p ${KICKER_HEADER_OUTPUT} && python3 ${CONVERT_SCRIPT} kicker/build/bin/kicker.nib ${KICKER_HEADER_OUTPUT}/kicker_bin.h KICKER_BYTES
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)
