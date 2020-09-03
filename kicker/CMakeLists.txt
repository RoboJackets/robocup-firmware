set( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/attiny_toolchain.cmake )
include( ${CMAKE_TOOLCHAIN_FILE})

project(kicker LANGUAGES C)

# include(
#     ${CMAKE_CURRENT_SOURCE_DIR}/attiny_toolchain.cmake
# )

# set(CMAKE_C_COMPILER    ${AVR_C_COMPILER})
# set(CMAKE_CXX_COMPILER  ${AVR_CXX_COMPILER})
# set(CMAKE_OBJCOPY  ${AVR_OBJCOPY})

message("----------------------CMAKE_C_COMPILER ${CMAKE_C_COMPILER}" )
message("----------------------CMAKE_OBJCOPY ${CMAKE_OBJCOPY}" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/device-bins" )

# compile executable for kicker firmware
add_executable(kicker.elf
    src/main.c
    src/HAL_attiny167.c
)

target_include_directories(kicker.elf PUBLIC
    include
)

# NOTE control should be dependent on this target and I think the includes should transfer through to the control target

# custom target to convert kicker.elf to the kickerFW binary file
add_custom_target(kicker ALL
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    COMMAND ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    COMMAND python3 ${CONVERT_SCRIPT} kicker.nib kicker_bin.h KICKER_BYTES
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)
