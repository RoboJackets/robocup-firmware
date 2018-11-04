project(STM32F7xx_lib LANGUAGES C ASM)

add_library(STM32F7xx
    startup.s
    mtrain/mtrain.c
    mtrain/stm32f7xx_it.c
    mtrain/stm32f7xx_hal_msp.c
    mtrain/usb/usbd_cdc_interface.c
)

target_include_directories(STM32F7xx PUBLIC
    mtrain
    mtrain/usb
)

target_link_libraries(STM32F7xx PUBLIC DSP STM32F7xx_HAL STM32_USB_Device)
