project(STM32F7xx_HAL LANGUAGES C)

add_subdirectory(CMSIS/DSP_Lib)
add_subdirectory(middleware/FreeRTOS)

aux_source_directory(STM32F7xx_HAL_Drivers/Src STM32F7xx_HAL_SRC)

add_library(STM32F7xx_HAL
    ${STM32F7xx_HAL_SRC}
    CMSIS/Device/STM32F7xx/Src/system_stm32f7xx.c
)

target_include_directories(STM32F7xx_HAL PUBLIC
    STM32F7xx_HAL_Drivers/Inc
    CMSIS/Device/STM32F7xx/Include
    CMSIS/Include
)

# Include HAL configuration from BSP
target_compile_definitions(STM32F7xx_HAL PUBLIC USE_HAL_DRIVER)
target_include_directories(STM32F7xx_HAL PUBLIC ${CMAKE_SOURCE_DIR}/BSP/config/hal)

target_link_libraries(STM32F7xx_HAL PUBLIC DSP)

add_subdirectory(middleware/STM32_USB_Device_Library)