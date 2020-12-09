cmake_minimum_required(VERSION 3.4)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/mtrain-toolchain.cmake)

project(
    mTrain
    LANGUAGES C CXX ASM
)

set(FLASH_COPY_SCRIPT python3 ${CMAKE_CURRENT_LIST_DIR}/util/flash.py)

add_subdirectory(external)

add_subdirectory(API/c)
add_subdirectory(API/cpp)
add_subdirectory(BSP)

add_library(mtrain INTERFACE)

target_link_libraries(mtrain INTERFACE
    STM32F7xx
    STM32F7_API_C
    STM32F7_API_CPP
)

if( IS_DIRECTORY  "${CMAKE_CURRENT_LIST_DIR}/tests" )
    add_subdirectory(tests/c)
    add_subdirectory(tests/cpp)
endif()
