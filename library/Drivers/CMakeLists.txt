project(STM32F7xx_lib LANGUAGES C ASM)

aux_source_directory(STM32F7xx_HAL_Driver/Src STM32F7xx_HAL_SRC)

add_library(STM32F7xx
    ${STM32F7xx_HAL_SRC}
    startup_stm32f769xx.s
    BSP/system_stm32f7xx.c
)

target_include_directories(STM32F7xx PUBLIC
    STM32F7xx_HAL_Driver/Inc
    CMSIS/Device/STM32F7xx/Include
    CMSIS/Include
    BSP
)

add_subdirectory(CMSIS/DSP_Lib)
target_link_libraries(STM32F7xx PUBLIC DSP)
