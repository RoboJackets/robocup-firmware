project(STM32F7xx_lib LANGUAGES C ASM)

add_library(STM32F7xx
    startup.s
    mtrain/mtrain.c
)

target_include_directories(STM32F7xx PUBLIC
    mtrain
)

target_link_libraries(STM32F7xx PUBLIC DSP STM32F7xx_HAL)
