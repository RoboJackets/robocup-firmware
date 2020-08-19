# Set your project name
project(mtrain LANGUAGES C CXX ASM)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/mjackets-api/cmake/stm32-gcc-arm-none-eabi.cmake)

# Set chip model so mjackets knows what to build
set(CHIP STM32F7 CACHE STRING "Full STM32 Chip Model")
add_definitions(-DSTM32F769NI)
add_definitions(-DSTM32F7)

add_subdirectory(mjackets-api)
add_subdirectory(bsp)

add_library(${CMAKE_PROJECT_NAME})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC bsp/)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC API)

STM32_ADD_HEX_BIN_TARGETS(${CMAKE_PROJECT_NAME})
