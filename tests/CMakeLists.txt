project(Test LANGUAGES C)

function(add_test test_file)
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name}.elf EXCLUDE_FROM_ALL
        ${test_file}
        # ${CMAKE_SOURCE_DIR}/BSP/Src/stm32f7xx_it.c #TODO: Fix
    )

    target_link_libraries(${test_name}.elf STM32F7xx)

    add_custom_target(test-${test_name}
        arm-none-eabi-objcopy -Obinary "${test_name}.elf" "${test_name}.bin"
        DEPENDS ${test_name}.elf
        COMMENT "objcopying to make binary executable")

    add_custom_target(test-${test_name}-prog
        COMMAND ${FLASH_COPY_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.bin
        DEPENDS test-${test_name}
        COMMENT "Copying binary to MCU")
endfunction()

# TODO: better way to do this without redefining every test
add_test("blink.c")
add_test("flash.c")
add_test("usb_serial.c")