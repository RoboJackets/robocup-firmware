cmake_minimum_required(VERSION 3.14)
project(BSP LANGUAGES C CXX ASM)

set(BSP_PATH ${CMAKE_CURRENT_SOURCE_DIR})

string(TOLOWER ${FAMILY} FAMILY_L)
string(TOLOWER ${TYPE} TYPE_L)

find_path(BSP_INCLUDE_PATH
    NAMES stm32${FAMILY_L}xx_hal_conf.h
    PATHS "${BSP_PATH}" 
    NO_DEFAULT_PATH
    REQUIRED
)

add_library(BSP INTERFACE)
target_include_directories(BSP INTERFACE "${BSP_INCLUDE_PATH}")
target_link_libraries(BSP INTERFACE API)

