project(STM32F7_API_C_lib LANGUAGES C)

aux_source_directory(Src/ API_C_SRC)

add_library(STM32F7_API_C
    ${API_C_SRC}
)

target_include_directories(STM32F7_API_C PUBLIC
    Inc/
    ./
)

target_link_libraries(STM32F7_API_C PUBLIC STM32F7xx_HAL)