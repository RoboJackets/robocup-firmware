file(GLOB SRC "Src/*.c")

add_library(STM32F7xx
    startup.s
    Src/bsp.c
    Src/interrupt_handlers.c
    Src/default_handlers.c
    Src/stm32f7xx_hal_msp.c
    Src/usbd_cdc_interface.c
    Src/qspi.c
    Src/rtos_hooks.c
)

target_include_directories(STM32F7xx PUBLIC
    Inc
)

set_target_properties(STM32F7xx PROPERTIES LINK_DEPENDS "${PROJECT_SOURCE_DIR}/BSP/flash.ld")

target_link_libraries(STM32F7xx PUBLIC STM32F7xx_HAL STM32_USB_Device FreeRTOS)
