project(STM32_USB_Device_lib LANGUAGES C)

aux_source_directory(Core/Src USB_DEVICE_CORE_SRC)

add_library(STM32_USB_Device
    ${USB_DEVICE_CORE_SRC}
    Class/CDC/Src/usbd_cdc.c
)

target_include_directories(STM32_USB_Device PUBLIC
    Core/Inc
    Class/CDC/Inc
)

# Include HAL configuration from BSP
target_include_directories(STM32_USB_Device PUBLIC ${CMAKE_SOURCE_DIR}/BSP/config/usb)

target_sources(STM32_USB_Device PUBLIC
    ${CMAKE_SOURCE_DIR}/BSP/config/usb/usbd_conf.c
    ${CMAKE_SOURCE_DIR}/BSP/config/usb/usbd_desc.c
)


target_link_libraries(STM32_USB_Device PUBLIC STM32F7xx_HAL)