# Set your project name
project(mtrain LANGUAGES C CXX ASM)

# Set chip model so mjackets knows what to build
set(CHIP STM32F769NI CACHE STRING "Full STM32 Chip Model")

add_subdirectory(mjackets-api)
add_subdirectory(bsp)

add_library(mtrain)

target_link_libraries(mtrain PUBLIC API)
