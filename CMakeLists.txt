cmake_minimum_required(VERSION 3.8)
project("robocup-firmware")


set(CONVERT_SCRIPT ${CMAKE_SOURCE_DIR}/convert.py)
set(FLASH_COPY_SCRIPT python3 ${PROJECT_SOURCE_DIR}/util/flash.py)


# run all the other CMakeLists files
add_subdirectory(kicker)
add_subdirectory(fpga)
add_subdirectory(control)
