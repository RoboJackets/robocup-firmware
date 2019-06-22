cmake_minimum_required(VERSION 3.4)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )

# compile executable for kicker firmware
add_executable(kicker.elf
    main.c
    HAL_atmega32a.c
)

target_include_directories(kicker.elf PUBLIC
    ${PROJECT_SOURCE_DIR}/../robot/lib/Inc/drivers/Internal
)

# custom target to convert kicker.elf to the kickerFW binary file and place it in the 'run' directory
add_custom_target(kicker ALL
    # the -j options tell objcopy what sections to include in the output
    # the -O ihex specifies what output format to use, which is the hex format needed for flashing the AVR
    ${CMAKE_OBJCOPY} -j .text -j .data -O binary kicker.elf kicker.nib
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS kicker.elf
    COMMENT "objcopying to convert kicker.elf to kicker.nib"
)

# # add command to copy the kicker firmware onto the mbed
# add_custom_target(kicker-upload
#     COMMAND ${MBED_COPY_SCRIPT} kicker.nib
#     WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
#     DEPENDS kicker
#     COMMENT "Copying the kicker board's binary over to the mbed"
# )
