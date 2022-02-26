cmake_minimum_required(VERSION 3.16.3)

# set the AVR toolchain
set( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/atmega-toolchain.cmake )

project(kicker LANGUAGES C)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )

# compile executable for kicker firmware
add_executable(kicker.elf
    src/main.c
)

add_executable(kicker-test.elf
    src/test.c
)

target_include_directories(kicker.elf PUBLIC
    include
)

target_include_directories(kicker-test.elf PUBLIC
    include
)

# custom target to convert kicker.elf to the kicker.nib binary file and place it in the 'run' directory
add_custom_target(kicker ALL
    ${AVR_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker.elf
    COMMENT "Objcopying to convert kicker.elf to kicker.nib"
)

# custom target to convert kicker-test.elf to the kicker-test.nib binary file and place it in the 'run' directory
add_custom_target(kicker-test ALL
    ${AVR_OBJCOPY} -j .text -j .data -O binary kicker-test.elf kicker-test.nib
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker-test.elf
    COMMENT "Objcopying to convert kicker-test.elf to kicker-test.nib"
)
