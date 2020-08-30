cmake_minimum_required(VERSION 3.14.7)

project(control
    LANGUAGES C CXX
)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/mjackets-api/cmake/stm32-gcc-arm-none-eabi.cmake)

set(CMAKE_CXX_COMPILER_VERSION "7.3")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin" )

add_definitions(-Wall)

# TODO: remove
add_definitions(-Wno-register)

add_subdirectory(mtrain)
add_subdirectory(external)

add_executable(control.elf
    src/robocup/main.cpp
    src/robocup/radio/RadioLink.cpp
    src/robocup/modules/BatteryModule.cpp
    src/robocup/modules/FPGAModule.cpp
    src/robocup/modules/IMUModule.cpp
    src/robocup/modules/KickerModule.cpp
    src/robocup/modules/LEDModule.cpp
    src/robocup/modules/MotionControlModule.cpp
    src/robocup/modules/RadioModule.cpp
    src/robocup/modules/RotaryDialModule.cpp
    src/robocup/motion-control/DribblerController.cpp
    src/robocup/motion-control/RobotController.cpp
    src/robocup/motion-control/RobotEstimator.cpp
    src/drivers/AVR910.cpp
    src/drivers/Battery.cpp
    src/drivers/FPGA.cpp
    src/drivers/I2Cdev.cpp
    src/drivers/ISM43340.cpp
    src/drivers/KickerBoard.cpp
    src/drivers/MCP23017.cpp
    src/drivers/MPU6050.cpp
)

## Shouldnt need explicit inclusion since mtrain dev relies on these
target_include_directories(control.elf PUBLIC
	include
    ${KICKER_HEADER_OUTPUT}
    # include/robocup
)

target_link_libraries(control.elf
    # By linking against rc-fshare it should already have eigen
    rc-fshare
    mtrain
)

add_custom_target(control ALL
    arm-none-eabi-objcopy -Obinary "control.elf" "control.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS control.elf
    COMMENT "objcopying to make binary executable for control"
)

add_custom_target(control-upload
    COMMAND $ENV{FLASH_COPY_SCRIPT} "control.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS control
    COMMENT "Copying binary control to MCU"
)

add_subdirectory(test)
