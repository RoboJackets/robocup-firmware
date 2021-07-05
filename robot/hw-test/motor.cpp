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
#include "drivers/MCP23017.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include <LockedStruct.hpp>
#include <Micropackets.hpp>
#include "MicroPackets.hpp"
#include "drivers/MCP23017.hpp"
#include "drivers/RotarySelector.hpp"
#include "drivers/IOExpanderDigitalInOut.hpp"

DebugInfo debugInfo;

int main() {

    MotorCommand motorCommand;
    //auto motorCommandLock = motorCommand.lock();
    motorCommand.isValid = false;
    motorCommand.lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorCommand.wheels[i] = 0;
    }
    motorCommand.dribbler = 0;

    FPGAStatus fpgaStatus;
    //auto fpgaStatusLock = fpgaStatus.lock();
    MotorFeedback motorFeedback;
    //auto motorFeedbackLock = motorFeedback.lock();
    RobotID robotID;
    //auto robotIDLock = robotID.lock();
    //LockedStruct<RobotID>& robotIDPtr = robotID;

    DigitalOut led1(LED1);
    DigitalOut led2(LED2);

    //LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);
    std::unique_ptr<SPI> fpgaKickerSPI = std::make_unique<SPI>(FPGA_SPI_BUS, std::nullopt, 16'000'000);
    //LockedStruct<MCP23017> ioExpander(MCP23017{sharedI2C, 0x42});
    //std::shared_ptr<I2C> sharedI2C = std::make_shared<I2C>(SHARED_I2C_BUS);
    //std::shared_ptr<MCP23017>
    //std::shared_ptr<MCP23017> ioExpander = std::make_shared<MCP23017>(sharedI2C, 0x42);
    // ioExpander.config(0x00FF, 0x00FF, 0x00FF);

    RotarySelector<DigitalIn> dial({p25, p26, p27, p28});
    FPGA fpga(std::move(fpgaKickerSPI), p31, p14, p13, p15);


    // Get initial dial value
    dial.init();
    dial.read();

    while (robotID.robotID != 0) {
        dial.read();
        HAL_Delay(100);
    }

    led1 = 1;

    while (true) {
        dial.read();
        printf("RobotID: %d\r\n", robotID.robotID);

        float duty = ((robotID.robotID ^ 8) - 8) % 8 / 8.0;

        motorCommand.isValid = true;
        motorCommand.lastUpdate = HAL_GetTick();
        for (int i = 0; i < 4; i++) {
            motorCommand.wheels[i] = duty;
        }
        motorCommand.dribbler = abs(duty*127);
        int16_t duty_cycles[5] = {500, 500, 500, 500, 500};
        size_t length = 5;
        fpga.set_duty_cycles(duty_cycles, length);
        HAL_Delay(100);
    }
}
