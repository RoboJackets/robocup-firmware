#include "mtrain.hpp"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modules/GenericModule.hpp"

#include "SPI.hpp"
#include "I2C.hpp"
#include "delay.h"
#include "DigitalOut.hpp"

#include <unistd.h>

#include "MicroPackets.hpp"
#include "iodefs.h"

#include "modules/BatteryModule.hpp"
#include "modules/FPGAModule.hpp"
#include "modules/IMUModule.hpp"
#include "modules/KickerModule.hpp"
#include "modules/LEDModule.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include "LockedStruct.hpp"

DebugInfo debugInfo;
// turn RobotID into a motor command
int main() {
    std::shared_ptr<SPI> fpgaKickerSPI = std::make_shared<SPI>(FPGA_KICKER_SPI_BUS, std::nullopt, 16'000'000);
    std::shared_ptr<I2C> sharedI2C = std::make_shared<I2C>(SHARED_I2C_BUS);
    std::shared_ptr<MCP23017> ioExpander = std::make_shared<MCP23017>(sharedI2C, 0x42);
    ioExpander->config(0x00FF, 0x00FF, 0x00FF);

    RotaryDialModule dial = new RotaryDialModule(LockedStruct<MCP23017>ioExpander, LockedStruct<RobotID> robotID);
    dial.start();

    {
        auto motorCommandLock = LockedStruct<MotorCommand> motorCommand;
        motorCommandLock = motorCommand.lock();
        motorCommandLock->isValid = false;
        motorCommandLock->lastUpdate = 0;
        for (int i = 0; i < 4; i++) {
            motorCommandLock.wheels[i] = 0;
        }
        motorCommandLock->dribbler = 0;
    }

    DigitalOut led1(LED1);
    DigitalOut led2(LED2);

    FPGAStatus fpgaStatus;
    MotorFeedback motorFeedback;
    auto robotIDLock = robotID.lock();

    FPGA fpga = new  FPGAModule(std::unique_ptr<SPI> spi,
                  LockedStruct<MotorCommand>& motorCommand,
                  LockedStruct<FPGAStatus>& fpgaStatus,
                  LockedStruct<MotorFeedback>& motorFeedback)
    fpga.start();

    while (robotIDLock.robotID != 0) {
        //gets intiial dial value
        dial.entry();
        HAL_Delay(100);
    }

    led1 = 1;

    while (true) {
        dial.entry();

        printf("RobotID: %d\r\n", robotIDLock.robotID);

        float duty = ((robotIDLock.robotID ^ 8) - 8) % 8 / 8.0;

        motorCommandLock->isValid = true;
        motorCommandLock->lastUpdate = HAL_GetTick();
        for (int i = 0; i < 4; i++) {
            motorCommandLock->wheels[i] = duty;
        }
        motorCommandLock->dribbler = abs(duty*127);
        //compile error that entry is not a method of fpga class
        // but an entry method is define in the FPGAModule code
        fpga.entry();
        HAL_Delay(100);
    }
}






   /*
        IODIR = 0x00,
        IPOL = 0x02,
        GPINTEN = 0x04,
        DEFVAL = 0x06,
        INTCON = 0x08,
        IOCON = 0x0A,
        GPPU = 0x0C,
        INTF = 0x0E,
        INTCAP = 0x10,
        GPIO = 0x12,
        OLAT = 0x14
    */

   /*
    #define IODIR 0x00
    #define IPOL 0x02
    #define GPPU 0x0C
    #define IO_GPIOA 0x12
    #define HEX_SWITCH_BIT0 7
    #define HEX_SWITCH_BIT1 4
    #define HEX_SWITCH_BIT2 6
    #define HEX_SWITCH_BIT3 5
    #define i2cAddr 0x42
    uint8_t configData = 0xFF;
    sharedI2C->transmit(i2cAddr, IODIR, configData);
    sharedI2C->transmit(i2cAddr, IPOL, configData);
    sharedI2C->transmit(i2cAddr, GPPU, configData);
    while (true) {
        uint8_t val = sharedI2C->receive(i2cAddr, IO_GPIOA);
        uint8_t rotary = (val >> HEX_SWITCH_BIT0) & 0x1;
        rotary |= ((val >> HEX_SWITCH_BIT1) & 0x1) << 1;
        rotary |= ((val >> HEX_SWITCH_BIT2) & 0x1) << 2;
        rotary |= ((val >> HEX_SWITCH_BIT3) & 0x1) << 3;
        uint8_t rotaryInv = !((val >> HEX_SWITCH_BIT0) & 0x1);
        rotaryInv |= (!((val >> HEX_SWITCH_BIT1) & 0x1)) << 1;
        rotaryInv |= (!((val >> HEX_SWITCH_BIT2) & 0x1)) << 2;
        rotaryInv |= (!((val >> HEX_SWITCH_BIT3) & 0x1)) << 3;
        printf("Rotary knob %d\r\n", rotary);
    }
    */
