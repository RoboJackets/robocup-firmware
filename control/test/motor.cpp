#include <LockedStruct.hpp>

#include "I2C.hpp"
#include "MicroPackets.hpp"
#include "SPI.hpp"
#include "drivers/IOExpanderDigitalInOut.hpp"
#include "drivers/MCP23017.hpp"
#include "drivers/RotarySelector.hpp"
#include "iodefs.h"
#include "modules/FPGAModule.hpp"
#include "modules/LEDModule.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include "mtrain.hpp"

int main() {
    LockedStruct<MotorCommand> motorCommand{};
    auto motorCommandLock = motorCommand.lock();
    motorCommandLock->isValid = false;
    motorCommandLock->lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorCommandLock->wheels[i] = 0;
    }
    motorCommandLock->dribbler = 0;

    LockedStruct<FPGAStatus> fpgaStatus{};
    auto fpgaStatusLock = fpgaStatus.lock();
    LockedStruct<MotorFeedback> motorFeedback{};
    auto motorFeedbackLock = motorFeedback.lock();
    LockedStruct<RobotID> robotID;
    auto robotIDLock = robotID.lock();
    // LockedStruct<RobotID>& robotIDPtr = robotID;

    DigitalOut led1(LED1);
    DigitalOut led2(LED2);

    LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);
    std::unique_ptr<SPI> fpgaKickerSPI =
        std::make_unique<SPI>(FPGA_SPI_BUS, std::nullopt, 16'000'000);
    LockedStruct<MCP23017> ioExpander(MCP23017{sharedI2C, 0x42});
    // std::shared_ptr<I2C> sharedI2C = std::make_shared<I2C>(SHARED_I2C_BUS);
    // std::shared_ptr<MCP23017>
    // std::shared_ptr<MCP23017> ioExpander = std::make_shared<MCP23017>(sharedI2C, 0x42);
    // ioExpander.config(0x00FF, 0x00FF, 0x00FF);

    RotaryDialModule dial(ioExpander, robotID);
    FPGAModule fpga(std::move(fpgaKickerSPI), motorCommand, fpgaStatus, motorFeedback);

    // Get initial dial value
    dial.entry();

    while (robotIDLock->robotID != 0) {
        dial.entry();
        HAL_Delay(100);
    }

    led1 = 1;

    while (true) {
        dial.entry();
        printf("RobotID: %d\r\n", robotIDLock->robotID);

        // what is going on here?
        float duty = ((robotIDLock->robotID ^ 8) - 8) % 8 / 8.0;

        motorCommandLock->isValid = true;
        motorCommandLock->lastUpdate = HAL_GetTick();
        for (int i = 0; i < 4; i++) {
            motorCommandLock->wheels[i] = duty;
        }
        motorCommandLock->dribbler = abs(duty * 127);
        fpga.entry();
        HAL_Delay(100);
    }
}
