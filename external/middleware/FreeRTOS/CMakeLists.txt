cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_C_STANDARD 11 CACHE STRING "")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "")

project(FreeRTOS_lib LANGUAGES C)

file(GLOB_RECURSE SRC *.c)

add_library(FreeRTOS STATIC
    ${SRC}
)

target_include_directories(FreeRTOS PUBLIC
    Source/include
    Source/CMSIS_RTOS
    Source/portable/GCC/ARM_CM7/r0p1
)

target_link_libraries(FreeRTOS STM32F7xx)
