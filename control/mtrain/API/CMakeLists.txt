aux_source_directory(Src/ API_SRC)

add_library(STM32F7_API
    ${API_SRC}
)

target_include_directories(STM32F7_API PUBLIC
    Inc/
)

target_link_libraries(STM32F7_API PUBLIC STM32F7xx_HAL)
