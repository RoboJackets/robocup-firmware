project(STM32F7xx_lib LANGUAGES C ASM)

aux_source_directory(../external/STM32F7xx_HAL_Drivers/Src STM32F7xx_HAL_SRC)

add_library(STM32F7xx
    ${STM32F7xx_HAL_SRC}
    startup_stm32f769xx.s
    system_stm32f7xx.c
)

target_include_directories(STM32F7xx PUBLIC
    ../external/STM32F7xx_HAL_Drivers/Inc
    ../external/CMSIS/Device/STM32F7xx/Include
    ../external/CMSIS/Include
    .
)

add_subdirectory(../external/CMSIS/DSP_Lib "${CMAKE_CURRENT_BINARY_DIR}/external/CMSIS/DSP_Lib")
target_link_libraries(STM32F7xx PUBLIC DSP)
