add_executable( base.elf
    main.cpp
)
target_link_libraries(base.elf firm-lib)

add_custom_target(base ALL
    arm-none-eabi-objcopy -Obinary "base.elf" "base.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS base.elf
    COMMENT "objcopying to make binary executable for base"
)

add_custom_target(upload-base
    COMMAND $ENV{FLASH_COPY_SCRIPT} "base.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS base
    COMMENT "Copying binary base to MCU"
)
