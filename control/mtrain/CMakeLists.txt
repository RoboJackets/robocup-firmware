# NOTE CMAKE_CURRENT_SOURCE_DIR should point to /robocup-firmware/control
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/mjackets-api/cmake/stm32-gcc-arm-none-eabi.cmake)

# Set your project name
project(mtrain LANGUAGES C CXX ASM)
# include(${CMAKE_TOOLCHAIN_FILE})
# # set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
# #     -T '${CMAKE_SOURCE_DIR}/bsp/flash.ld'")

# Set chip model so mjackets knows what to build
set(CHIP STM32F769NI CACHE STRING "Full STM32 Chip Model")

add_subdirectory(mjackets-api)
add_subdirectory(bsp)

add_library(mtrain)

target_include_directories(mtrain PUBLIC bsp/)

target_link_libraries(mtrain PUBLIC API)
