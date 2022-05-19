#include "common.hpp"
#include "task.h"
#include "core_cm7.h"

#include "modules/GenericModule.hpp"
#include "modules/BatteryModule.hpp"
#include "modules/FPGAModule.hpp"
#include "modules/IMUModule.hpp"
#include "modules/KickerModule.hpp"
#include "modules/LEDModule.hpp"
#include "modules/MotionControlModule.hpp"
#include "modules/RadioModule.hpp"
#include "modules/RotaryDialModule.hpp"
#include <vector>
#include <algorithm>
#include <cstdio>
#include <unistd.h>

#define SUPER_LOOP_FREQ 200
#define SUPER_LOOP_PERIOD (1000000L / SUPER_LOOP_FREQ)

// Max number of super loop cycles a proc can miss if it
// needs to run
#define MAX_MISS_CNT 3

struct MODULE_META_DATA {
    // Time in sysclock ticks of last module execution
    uint32_t lastRunTime;

    // Time in sysclock ticks between module executions
    const uint32_t modulePeriod;

    // Estimate in sysclock ticks of module runtime
    int32_t moduleRunTime;

    // Number of times the module has completed
    int32_t numCompletions;

    GenericModule *module;

    /**
     * Period and runtime in us
     */
    MODULE_META_DATA(uint64_t lastRunTime,
                     uint32_t modulePeriod,
                     int32_t moduleRunTime,
                     GenericModule *module)
            : lastRunTime(lastRunTime),
              modulePeriod(modulePeriod * DWT_SysTick_To_us()),
              moduleRunTime(moduleRunTime * DWT_SysTick_To_us()),
              numCompletions(0),
              module(module) {}
    /**
    * Period and runtime in us
    */
    explicit MODULE_META_DATA(GenericModule *module)
            : lastRunTime(0),
            modulePeriod(module->period.count()), moduleRunTime(0), numCompletions(0),
            module(module) {}
};

[[noreturn]]
void startModule(void *pvModule) {
    GenericModule *module = static_cast<GenericModule *>(pvModule);

    printf("[INFO] Starting module %s\r\n", module->name);
    module->start();
    printf("[INFO] Finished starting module %s\r\n", module->name);

    TickType_t last_wait_time = xTaskGetTickCount();
    TickType_t increment = module->period.count();

    while (true) {
        module->entry();
        vTaskDelayUntil(&last_wait_time, increment);
    }
}

std::vector<const char*> failed_modules;
size_t free_space;
// Map name of modules to their metadata
static std::unordered_map<const char *, MODULE_META_DATA> modules;

/**
 * Wrapper for freeRTOS xTaskCreate
 * @param module pointer to the module to create a task for
 * @return whether or not the creation was successful
 */
bool createModule(GenericModule *module) {
    BaseType_t result = xTaskCreate(startModule,
                                    module->name,
                                    module->stackSize,
                                    module,
                                    module->priority,
                                    &(module->handle));
    if (result != pdPASS) {
        printf("[ERROR] Failed to initialize task %s for reason %x:\r\n", module->name, module->stackSize);
        failed_modules.push_back(module->name);
        return false;
    } else {
        printf("[INFO] Initialized task %s.\r\n", module->name);
        return true;
    }
}

[[ noreturn ]] int main() {
    // Delay for USB startup
    DWT_Delay(2500);

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

    static LockedStruct<MCP23017> ioExpander(MCP23017{sharedI2C, 0x42});

    printf("[INFO] Starting up.\r\n");

    static LEDModule led(ioExpander,
                         sharedSPI,
                         batteryVoltage,
                         fpgaStatus,
                         kickerInfo,
                         radioError,
                         imuData);


    static BatteryModule battery(batteryVoltage);


    static RotaryDialModule dial(ioExpander,
                                 robotID);

    static IMUModule imu(sharedSPI, imuData);

    static RadioModule radio(batteryVoltage,
                             fpgaStatus,
                             kickerInfo,
                             robotID,
                             kickerCommand,
                             motionCommand,
                             radioError,
                             debugInfo);

    static KickerModule kicker(sharedSPI,
                               kickerCommand,
                               kickerInfo);

    static FPGAModule fpga(std::move(fpgaSPI),
                           motorCommand,
                           fpgaStatus,
                           motorFeedback);

    static MotionControlModule motion(batteryVoltage,
                                      imuData,
                                      motionCommand,
                                      motorFeedback,
                                      motorCommand,
                                      debugInfo);

    modules =
            {{led.name,MODULE_META_DATA(&led)},
             {dial.name, MODULE_META_DATA(&dial)},
             {imu.name, MODULE_META_DATA(&imu)},
             {radio.name, MODULE_META_DATA(&radio)},
             {kicker.name, MODULE_META_DATA(&kicker)},
             {fpga.name, MODULE_META_DATA(&fpga)},
             {motion.name, MODULE_META_DATA(&motion)}};

    for (const auto [ name, data ] : modules) {
        if (!createModule(data.module)) {
            NVIC_SystemReset();
        }
    }

    ////////////////////////////////////////////

    printf("[INFO] Starting scheduler!\r\n");

    vTaskStartScheduler();

    printf("[ERROR] Failed to start scheduler!\r\n");

    for (;;) {}
}
