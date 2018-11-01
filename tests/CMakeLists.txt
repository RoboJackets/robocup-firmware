project(Test LANGUAGES C)

function(add_test test_file)
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name}.elf
        ${test_file}
    )

    target_link_libraries(${test_name}.elf STM32F7xx)

    add_custom_target (${test_name}.bin ALL arm-none-eabi-objcopy -Obinary "${test_name}.elf" "${test_name}.bin" DEPENDS ${test_name}.elf)
endfunction()

add_test("blink.c")