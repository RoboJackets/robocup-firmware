cmake_minimum_required(VERSION 3.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/mtrain/cmake/mtrain-toolchain.cmake)

project(
    control
    LANGUAGES ASM C CXX
)

add_subdirectory(mtrain)
add_subdirectory(external)

add_library(control-lib
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

add_custom_target(control-upload
    COMMAND ${FLASH_COPY_SCRIPT} "control.bin"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS control
    COMMENT "Copying binary control to MCU"
)

add_subdirectory(test)
