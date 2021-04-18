#include "mtrain.hpp"
#include "SPI.hpp"
#include "I2C.hpp"

#include "iodefs.h"
// #include "modules/GenericModule.hpp"
// #include "modules/BatteryModule.hpp"
#include "modules/FPGAModule.hpp"
// #include "modules/IMUModule.hpp"
// #include "modules/KickerModule.hpp"
#include "modules/LEDModule.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include <LockedStruct.hpp>
#include <Micropackets.hpp>

DebugInfo debugInfo;

int main() {

    LockedStruct<MotorCommand>& motorCommand;
    auto motorCommandLock = motorCommand.lock();
    motorCommandLock.isValid = false;
    motorCommandLock.lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorCommandLock.wheels[i] = 0;
    }
    motorCommandLock.dribbler = 0;

    LockedStruct<FPGAStatus>& fpgaStatus;
    LockedStruct<MotorFeedback>& motorFeedback;
    LockedStruct<RobotID>& robotID;
    auto robotIDLock = robotID.lock();

    DigitalOut led1(LED1);
    DigitalOut led2(LED2);

    std::shared_ptr<SPI> fpgaKickerSPI = std::make_shared<SPI>(FPGA_KICKER_SPI_BUS, std::nullopt, 16'000'000);
    std::shared_ptr<I2C> sharedI2C = std::make_shared<I2C>(SHARED_I2C_BUS);

    std::shared_ptr LockedStruct<MCP23017>& ioExpander = std::make_shared<MCP23017>(sharedI2C, 0x42);

    ioExpander->config(0x00FF, 0x00FF, 0x00FF);

    RotaryDialModule dial(ioExpander, robotIDLock);

    FPGAModule fpga(fpgaKickerSPI, &motorCommand, &fpgaStatus, &motorFeedback);


    // Get initial dial value
    dial.start();

    while (robotID->robotID != 0) {
        dial.start();
        HAL_Delay(100);
    }

    led1 = 1;

    while (true) {
        dial.start();
        auto motorCommandLock = motorCommand.lock();
        printf("RobotID: %d\r\n", robotIDLock->robotID);

        float duty = ((robotIDLock->robotID ^ 8) - 8) % 8 / 8.0;

        motorCommandLock->isValid = true;
        motorCommandLock->lastUpdate = HAL_GetTick();
        for (int i = 0; i < 4; i++) {
            motorCommandLock->wheels[i] = duty;
        }
        motorCommandLock->dribbler = abs(duty*127);
        fpga.entry();
        HAL_Delay(100);
    }
}
