add_executable(control.elf 
    main.cpp
)
target_link_libraries(control.elf firm-lib)

add_custom_target(control ALL
    arm-none-eabi-objcopy -Obinary "control.elf" "control.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS control.elf
    COMMENT "objcopying to make binary executable for control"
)

add_custom_target(control-upload
    COMMAND $ENV{FLASH_COPY_SCRIPT} "control.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS control
    COMMENT "Copying binary control to MCU"
)