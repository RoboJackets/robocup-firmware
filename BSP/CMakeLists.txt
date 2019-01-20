project(STM32F7xx_lib LANGUAGES C ASM)

add_library(STM32F7xx
    startup.s
    Src/bsp.c
    Src/interrupt_handlers.c
    Src/default_handlers.c
    Src/stm32f7xx_hal_msp.c
    Src/usbd_cdc_interface.c
    Src/qspi.c
)

target_include_directories(STM32F7xx PUBLIC
    Inc
)

target_link_libraries(STM32F7xx PUBLIC DSP STM32F7xx_HAL STM32_USB_Device)
