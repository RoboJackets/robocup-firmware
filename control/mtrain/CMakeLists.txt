# Set your project name
project(mtrain LANGUAGES C CXX ASM)

# Set chip model so mjackets knows what to build
set(CHIP STM32F769NI CACHE STRING "Full STM32 Chip Model")
add_definitions(-DSTM32F769NI)
add_definitions(-DSTM32F7)

add_subdirectory(mjackets-api)
add_subdirectory(bsp)

add_library(mtrain)

target_include_directories(mtrain PUBLIC bsp/)

target_link_libraries(mtrain PUBLIC API)

STM32_ADD_HEX_BIN_TARGETS(mtrain)
