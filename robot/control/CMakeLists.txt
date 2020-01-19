add_executable(control.elf 
    main.cpp
    Src/radio/RadioLink.cpp
    Src/modules/BatteryModule.cpp
    Src/modules/FPGAModule.cpp
    Src/modules/IMUModule.cpp
    Src/modules/KickerModule.cpp
    Src/modules/LEDModule.cpp
    Src/modules/MotionControlModule.cpp
    Src/modules/RadioModule.cpp
    Src/modules/RotaryDialModule.cpp
    Src/motion-control/DribblerController.cpp
    Src/motion-control/RobotController.cpp
    Src/motion-control/RobotEstimator.cpp
)

target_include_directories(control.elf PUBLIC
	Inc
	"${CONAN_MTRAIN_ROOT}/external/middleware/STM32_USB_Device_Library/Core/Inc/"
	"${CONAN_MTRAIN_ROOT}/external/middleware/STM32_USB_Device_Library/Class/CDC/Inc"
)

target_link_libraries(control.elf
    firm-lib
    CONAN_PKG::Eigen3
    rc-fshare
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
