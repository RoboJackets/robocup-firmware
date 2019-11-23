#include "mtrain.hpp"
#include "SPI.hpp"
#include "I2C.hpp"
#include "delay.h"
#include "DigitalOut.hpp"

#include <unistd.h>

#include "MicroPackets.hpp"
#include "iodefs.h"

#include "modules/GenericModule.hpp"
#include "modules/BatteryModule.hpp"
#include "modules/FPGAModule.hpp"
#include "modules/IMUModule.hpp"
#include "modules/KickerModule.hpp"
#include "modules/LEDModule.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include "LockedStruct.hpp"

#include "FreeRTOS.h"
#include "task.h"


#define SUPER_LOOP_FREQ 200
#define SUPER_LOOP_PERIOD (1000000L / SUPER_LOOP_FREQ)

// Max number of super loop cycles a proc can miss if it
// needs to run
#define MAX_MISS_CNT 5



struct MODULE_META_DATA {
    // Time in sysclock ticks of last module execution
    uint32_t lastRunTime;

    // Time in sysclock ticks of next module execution
    uint32_t nextRunTime;

    // Time in sysclock ticks between module executions
    const uint32_t modulePeriod;

    // Estimate in sysclock ticks of module runtime 
    const int32_t moduleRunTime;

    GenericModule* module;

    /**
     * Period and runtime in us
     */
    MODULE_META_DATA(uint64_t lastRunTime,
                     uint32_t modulePeriod,
                     int32_t moduleRunTime,
                     GenericModule* module)
        : lastRunTime(lastRunTime),
          nextRunTime(lastRunTime + modulePeriod*DWT_SysTick_To_us()),
          modulePeriod(modulePeriod*DWT_SysTick_To_us()),
          moduleRunTime(moduleRunTime*DWT_SysTick_To_us()),
          module(module) {}
};

static std::vector<GenericModule*> moduleList;

void startModule(void *pvModule) {
    GenericModule *module = static_cast<GenericModule *>(pvModule);

    TickType_t last_wait_time = xTaskGetTickCount();
    TickType_t increment = module->period.count();
    while (true) {
        module->entry();
        vTaskDelayUntil(&last_wait_time, increment);
    }
}

void createModule(GenericModule *module) {
    xTaskCreate(startModule, module->name, module->stackSize, module, module->priority, &(module->handle));
}

LockedStruct<DebugInfo> debugInfo;

int main() {
    HAL_Delay(100);

    LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);
    std::unique_ptr<SPI> fpgaSPI = std::make_unique<SPI>(FPGA_SPI_BUS, std::nullopt, 16'000'000);
    LockedStruct<SPI> sharedSPI(DOT_STAR_SPI_BUS, std::nullopt, 100'000);

    LockedStruct<MotionCommand> motionCommand;
    LockedStruct<MotorCommand> motorCommand;
    LockedStruct<MotorFeedback> motorFeedback;
    LockedStruct<IMUData> imuData;
    LockedStruct<BatteryVoltage> batteryVoltage;
    LockedStruct<FPGAStatus> fpgaStatus;
    LockedStruct<RadioError> radioError;
    LockedStruct<RobotID> robotID;
    LockedStruct<KickerCommand> kickerCommand;
    LockedStruct<KickerInfo> kickerInfo;

    // TODO: Fix me such that we init all the devices
    // then call a config to flash them correctly
    // this will allow us to force the CS lines into the correct
    // position before doing anything like flashing other
    // devices on the bus

    HAL_Delay(100);

//    std::shared_ptr<MCP23017> ioExpander = std::make_shared<MCP23017>(sharedI2C, 0x42);
    LockedStruct<MCP23017> ioExpander(MCP23017{sharedI2C, 0x42});

    ioExpander.unsafe_value()->config(0x00FF, 0x00FF, 0x00FF);

    LEDModule led(ioExpander,
                  batteryVoltage,
                  fpgaStatus,
                  kickerInfo,
                  radioError);
    createModule(&led);

    FPGAModule fpga(std::move(fpgaSPI),
                    motorCommand,
                    fpgaStatus,
                    motorFeedback);
    createModule(&fpga);

    RadioModule radio(batteryVoltage,
                      fpgaStatus,
                      kickerInfo,
                      robotID,
                      kickerCommand,
                      motionCommand,
                      radioError);
    createModule(&radio);

    KickerModule kicker(sharedSPI,
                        kickerCommand,
                        kickerInfo);
    createModule(&kicker);

    BatteryModule battery(batteryVoltage);
    createModule(&battery);

    RotaryDialModule dial(ioExpander,
                          robotID);
    createModule(&dial);

    MotionControlModule motion(batteryVoltage,
                               imuData,
                               motionCommand,
                               motorFeedback,
                               motorCommand);
    createModule(&motion);

//    IMUModule imu(sharedI2C,
//                  imuData);
//    createModule(&imu);

    // Start the scheduler now that we're done creating all of our tasks.
    vTaskStartScheduler();

    for (;;);
}