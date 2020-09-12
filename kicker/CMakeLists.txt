cmake_minimum_required(VERSION 3.14.7)

# set the AVR toolchain
set( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/attiny_toolchain.cmake )
# include(attiny_toolchain.cmake)
# set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
# set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})

project(kicker LANGUAGES C)

# message("---------------------- KICKER CMAKE_C_COMPILER ${CMAKE_C_COMPILER}" )
# message("---------------------- KICKER CMAKE_OBJCOPY ${CMAKE_OBJCOPY}" )

# compile executable for kicker firmware
add_executable(kicker.elf
    src/main.c
    src/HAL_attiny167.c
)

target_include_directories(kicker.elf PUBLIC
    include
)

# custom target to convert kicker.elf to the kickerFW binary file
add_custom_target(kicker all
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary ${CMAKE_BINARY_DIR}/kicker.elf kicker.nib
    COMMAND python3 ${CONVERT_SCRIPT} kicker.nib kicker_bin.h KICKER_BYTES
    WORKING_DIRECTORY "${SHARED_DIR}/devices-bin"
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)
