# Set your project name
project(mtrain LANGUAGES C CXX ASM)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -T '${CMAKE_SOURCE_DIR}/bsp/flash.ld'")

# Set chip model so mjackets knows what to build
set(CHIP STM32F769NI CACHE STRING "Full STM32 Chip Model")

add_subdirectory(mjackets-api)
add_subdirectory(bsp)

add_library(mtrain)

target_include_directories(mtrain PUBLIC bsp/)

target_link_libraries(mtrain PUBLIC API)

STM32_ADD_HEX_BIN_TARGETS(mtrain)
