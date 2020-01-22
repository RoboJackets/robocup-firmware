project(STM32F7_API_CPP_lib LANGUAGES CXX)

aux_source_directory(Src/ API_CPP_SRC)
aux_source_directory(../c/Src/ API_C_SRC)

add_library(STM32F7_API_CPP
    ${API_CPP_SRC}
    ${API_C_SRC}
)

target_include_directories(STM32F7_API_CPP PUBLIC
    Inc/
    ./
    ../c/Inc/
)

target_link_libraries(STM32F7_API_CPP PUBLIC STM32F7xx_HAL)