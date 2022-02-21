file(GLOB_RECURSE SRC "Src/*.cpp")

add_library(STM32F7xx
    startup.s
    ${SRC}
)

target_include_directories(STM32F7xx PUBLIC
    Inc
)

set_target_properties(STM32F7xx PROPERTIES LINK_DEPENDS "${PROJECT_SOURCE_DIR}/BSP/flash.ld")

target_link_libraries(STM32F7xx PUBLIC STM32F7xx_HAL STM32_USB_Device FreeRTOS)
