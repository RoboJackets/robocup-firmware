cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_C_STANDARD 11 CACHE STRING "")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/mtrain/cmake/mtrain-toolchain.cmake)

project(
    control
    LANGUAGES ASM C CXX
)

add_subdirectory(mtrain)
add_subdirectory(external)

file(GLOB_RECURSE SRC "src/*.cpp")

add_library(control-lib
    ${SRC}
)

## Shouldnt need explicit inclusion since mtrain dev relies on these
target_include_directories(control-lib PUBLIC
    include
    include/robocup
    # needs kicker commands
    ${CMAKE_CURRENT_LIST_DIR}/../kicker/include
    # needs kicker final binary for flashing
    ${CMAKE_CURRENT_LIST_DIR}/../kicker/build/bin
    # needs fpga final binary for flashing
    ${CMAKE_CURRENT_LIST_DIR}/../fpga/perm-bin
    # ${CMAKE_CURRENT_LIST_DIR}/../fpga/build/bin
)

target_link_libraries(control-lib
    # By linking against rc-fshare it should already have eigen
    rc-fshare
    mtrain
)

add_executable(control.elf
    src/robocup/main.cpp
)

target_link_libraries(control.elf
    control-lib
)

add_custom_target(control ALL
    arm-none-eabi-objcopy -Obinary "control.elf" "control.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS control.elf
    COMMENT "objcopying to make binary executable for control"
)

add_subdirectory(tests)
