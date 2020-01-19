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

MotionCommand motionCommand;
MotorCommand motorCommand;
MotorFeedback motorFeedback;
IMUData imuData;
BatteryVoltage batteryVoltage;
FPGAStatus fpgaStatus;
RadioError radioError;
RobotID robotID;
KickerCommand kickerCommand;
KickerInfo kickerInfo;

DebugInfo debugInfo;


int main() {
    HAL_Delay(100);

    std::shared_ptr<I2C> sharedI2C = std::make_shared<I2C>(SHARED_I2C_BUS);
    std::shared_ptr<SPI> fpgaKickerSPI = std::make_shared<SPI>(FPGA_KICKER_SPI_BUS, std::nullopt, 16'000'000);
    std::shared_ptr<SPI> dot_star_spi = std::make_shared<SPI>(DOT_STAR_SPI_BUS, std::nullopt, 100'000);

    // TODO: Fix me such that we init all the devices
    // then call a config to flash them correctly
    // this will allow us to force the CS lines into the correct
    // position before doing anything like flashing other
    // devices on the bus

    HAL_Delay(100);

    std::shared_ptr<MCP23017> ioExpander = std::make_shared<MCP23017>(sharedI2C, 0x42);
    ioExpander->config(0x00FF, 0x00FF, 0x00FF);

    // Init led first to show startup progress with LED's
    LEDModule led(ioExpander,
                  &batteryVoltage,
                  &fpgaStatus,
                  &kickerInfo,
                  &radioError);

    FPGAModule fpga(fpgaKickerSPI,
                    &motorCommand,
                    &fpgaStatus,
                    &motorFeedback);

    led.fpgaInitialized();

    RadioModule radio(&batteryVoltage,
                      &fpgaStatus,
                      &kickerInfo,
                      &robotID,
                      &kickerCommand,
                      &motionCommand,
                      &radioError);

    led.radioInitialized();

    // KickerModule kicker(dot_star_spi,
    //                     &kickerCommand,
    //                     &kickerInfo);

    // led.kickerInitialized();

    BatteryModule battery(&batteryVoltage);
    RotaryDialModule dial(ioExpander,
                          &robotID);
    MotionControlModule motion(&batteryVoltage,
                               &imuData,
                               &motionCommand,
                               &motorFeedback,
                               &motorCommand);
    // IMUModule imu(sharedI2C,
    //               &imuData);

    // led.fullyInitialized();


    std::vector<MODULE_META_DATA> moduleList;

    uint64_t curTime = DWT_GetTick();
    moduleList.emplace_back(curTime, MotionControlModule::period, MotionControlModule::runtime, &motion);
    // moduleList.emplace_back(curTime, IMUModule::period,           IMUModule::runtime,           &imu);
    moduleList.emplace_back(curTime, FPGAModule::period,          FPGAModule::runtime,          &fpga);
    moduleList.emplace_back(curTime, RadioModule::period,         RadioModule::runtime,         &radio);
    // moduleList.emplace_back(curTime, KickerModule::period,        KickerModule::runtime,        &kicker);
    moduleList.emplace_back(curTime, BatteryModule::period,       BatteryModule::runtime,       &battery);
    moduleList.emplace_back(curTime, RotaryDialModule::period,    RotaryDialModule::runtime,    &dial);
    moduleList.emplace_back(curTime, LEDModule::period,           LEDModule::runtime,           &led);

    while (true) {
        // Must do all timing in systick
        // Base time we use is in systick, and if you convert from systick
        // to us and try to compare timepoints, the value will "overflow" at
        // some random point which messes up the `Current - Start` magic
        // that automatically deals with delta time using the math behind overflows
        uint32_t loopStartTime = DWT_GetTick();
        uint32_t loopEndTime = DWT_GetTick() + SUPER_LOOP_PERIOD*DWT_SysTick_To_us();

        for (unsigned int i = 0; i < moduleList.size(); i++) {
            MODULE_META_DATA& module = moduleList.at(i);
            uint32_t currentTime = DWT_GetTick();

            // Check if we should run the module
            //      It's time to run it and
            //      we have enough time in the super loop
            //
            // Subtraction allows for rollover compensation
            // then convertion to signed allows simple comparison
            if (static_cast<int32_t>(currentTime - module.nextRunTime) >= 0 &&
                static_cast<int32_t>(loopEndTime - currentTime) >= module.moduleRunTime) {

                // todo change to loop start time
                module.lastRunTime = loopStartTime;
                module.nextRunTime = loopStartTime + module.modulePeriod;

                module.module->entry();
            }

            // Check if we missed a module X times in a row
            if (static_cast<int32_t>(currentTime - module.nextRunTime) > MAX_MISS_CNT*module.moduleRunTime) {
                //printf("WARNING: Missed module #%d run %d times in a row\r\n", i+1, MAX_MISS_CNT);
                led.missedModuleRun();
            }
        }

        int32_t elapsed = DWT_GetTick() - loopStartTime;
        if (elapsed < static_cast<int32_t>(SUPER_LOOP_PERIOD*DWT_SysTick_To_us())) {
            DWT_Delay_Sys(SUPER_LOOP_PERIOD*DWT_SysTick_To_us() - elapsed);
        } else {
            //printf("WARNING: Overran super loop time\r\n");
            led.missedSuperLoop();
        }

    }
}
