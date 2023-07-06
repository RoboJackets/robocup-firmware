#include <algorithm>
#include <cstdio>
#include <vector>

#include <unistd.h>

#include "FreeRTOS.h"
#include "I2C.hpp"
#include "LockedStruct.hpp"
#include "MicroPackets.hpp"
#include "SPI.hpp"
#include "delay.h"
#include "iodefs.h"
#include "modules/BatteryModule.hpp"
#include "modules/FPGAModule.hpp"
#include "modules/GenericModule.hpp"
#include "modules/IMUModule.hpp"
#include "modules/KickerModule.hpp"
#include "modules/LEDModule.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include "task.h"
#include "timer.h"

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

    GenericModule *module;

    /**
     * Period and runtime in us
     */
    MODULE_META_DATA(uint64_t lastRunTime,
                     uint32_t modulePeriod,
                     int32_t moduleRunTime,
                     GenericModule *module)
            : lastRunTime(lastRunTime),
              nextRunTime(lastRunTime + modulePeriod * DWT_us_To_SysTick()),
              modulePeriod(modulePeriod * DWT_us_To_SysTick()),
              moduleRunTime(moduleRunTime * DWT_us_To_SysTick()),
              module(module) {}
};

static std::vector<GenericModule *> moduleList;

[[noreturn]]
void startModule(void *pvModule) {
    GenericModule *module = static_cast<GenericModule *>(pvModule);

    printf("[INFO] Starting module %s\r\n", module->name);
    Start_TIM3(7);
    module->start();
    Stop_TIM3();
    printf("[INFO] Finished starting module %s\r\n", module->name);

    TickType_t last_wait_time = xTaskGetTickCount();
    TickType_t increment = module->period.count();

    while (true) {
        Start_TIM3(1);
        module->entry();
        Stop_TIM3();
        vTaskDelayUntil(&last_wait_time, increment);
    }
}

std::vector<const char*> failed_modules;
size_t free_space;

void createModule(GenericModule *module) {
    BaseType_t result = xTaskCreate(startModule,
                                    module->name,
                                    module->stackSize,
                                    module,
                                    module->priority,
                                    &(module->handle));
    if (result != pdPASS) {
        printf("[ERROR] Failed to initialize task %s for reason %x:\r\n", module->name, module->stackSize);
        failed_modules.push_back(module->name);
    } else {
        printf("[INFO] Initialized task %s.\r\n", module->name);
    }
}

int main() {
    static LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);
    static std::unique_ptr<SPI> fpgaSPI = std::make_unique<SPI>(FPGA_SPI_BUS, std::nullopt, 16'000'000);
    static LockedStruct<SPI> sharedSPI(SHARED_SPI_BUS, std::nullopt, 100'000);

    static LockedStruct<MotionCommand> motionCommand{};
    static LockedStruct<MotorCommand> motorCommand{};
    static LockedStruct<MotorFeedback> motorFeedback{};
    static LockedStruct<IMUData> imuData{};
    static LockedStruct<BatteryVoltage> batteryVoltage{};
    static LockedStruct<FPGAStatus> fpgaStatus{};
    static LockedStruct<RadioError> radioError{};
    static LockedStruct<RobotID> robotID{};
    static LockedStruct<KickerCommand> kickerCommand{};
    static LockedStruct<KickerInfo> kickerInfo{};
    static LockedStruct<DebugInfo> debugInfo{};
    static LockedStruct<LEDCommand> ledCommand;

    static LockedStruct<MCP23017> ioExpander(MCP23017{sharedI2C, 0x42});

    static LEDModule led(ioExpander,
                         sharedSPI,
                         batteryVoltage,
                         fpgaStatus,
                         kickerInfo,
                         radioError,
                         imuData,
                         ledCommand);
    createModule(&led);

    static IMUModule imu(sharedSPI, imuData);

    static FPGAModule fpga(std::move(fpgaSPI),
                           motorCommand,
                           fpgaStatus,
                           motorFeedback);
    createModule(&fpga);

    static RadioModule radio(batteryVoltage,
                             fpgaStatus,
                             kickerInfo,
                             robotID,
                             kickerCommand,
                             motionCommand,
                             radioError,
                             debugInfo,
                             ledCommand);
    createModule(&radio);

    static KickerModule kicker(sharedSPI,
                               kickerCommand,
                               kickerInfo);
    createModule(&kicker);

    static BatteryModule battery(batteryVoltage);
    createModule(&battery);

    static RotaryDialModule dial(ioExpander,
                                 robotID);
    createModule(&dial);

    static MotionControlModule motion(batteryVoltage, imuData, motionCommand, motorFeedback,
                                      motorCommand, debugInfo, imu);
    createModule(&motion);

    MX_TIM3_Init();

    ////////////////////////////////////////////

    printf("[INFO] Starting scheduler...\r\n");

    vTaskStartScheduler();

    printf("[ERROR] Failed to start scheduler!\r\n");

    for (;;) {}
}
