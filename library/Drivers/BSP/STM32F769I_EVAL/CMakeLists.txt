project(STM32F769I_EVAL_BSP LANGUAGES C)

aux_source_directory(src STM32F769I_EVAL_BSP_SRC)

add_library(STM32F769I_EVAL_BSP
    stm32f769i_eval_audio.c
    stm32f769i_eval_camera.c
    stm32f769i_eval_eeprom.c
    stm32f769i_eval_io.c
    # stm32f769i_eval_lcd.c
    stm32f769i_eval_nor.c
    stm32f769i_eval_qspi.c
    stm32f769i_eval_sd.c
    stm32f769i_eval_sdram.c
    stm32f769i_eval_sram.c
    # stm32f769i_eval_ts.c
    stm32f769i_eval.c
)

target_include_directories(STM32F769I_EVAL_BSP PUBLIC .)

target_compile_definitions(STM32F769I_EVAL_BSP PRIVATE USE_IOEXPANDER=1)
target_link_libraries(STM32F769I_EVAL_BSP STM32F7xx)
