cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_C_STANDARD 11 CACHE STRING "")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/mtrain-toolchain.cmake)

project(
    mTrain
    LANGUAGES C CXX ASM
)

add_subdirectory(external)

add_subdirectory(API)
add_subdirectory(BSP)
add_subdirectory(tests/cpp)

add_library(mtrain INTERFACE)

target_link_libraries(mtrain INTERFACE
    STM32F7xx
    STM32F7_API
)
