project(Test LANGUAGES C)

# add_executable(test.elf main.c)
# add_custom_target (test.bin ALL arm-none-eabi-objcopy -Obinary "test.elf" "test.bin" DEPENDS test.elf)

add_executable(test.elf
    main.c
)

target_include_directories(test.elf PUBLIC Inc)
target_link_libraries(test.elf STM32F7xx STM32F769I_EVAL_BSP)

add_custom_target (test.bin ALL arm-none-eabi-objcopy -Obinary "test.elf" "test.bin" DEPENDS test.elf)
