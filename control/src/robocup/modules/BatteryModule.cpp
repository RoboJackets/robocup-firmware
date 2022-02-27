#include "modules/BatteryModule.hpp"
#include "mtrain.hpp"
#include <memory>
#include "delay.h"

using namespace std::literals;

int BattCount; 
uint64_t BattTaskTime; 

BatteryModule::BatteryModule(LockedStruct<BatteryVoltage>& batteryVoltage)
    : GenericModule(1000ms, "battery"),
      batteryVoltage(batteryVoltage) {

    // It makes no sense to actually attempt to lock the mutex here, because
    // the scheduler hasn't started up yet. There's also no chance of preemption
    // while we're using it, because we're in startup, so just use the struct.
    auto batteryLock = batteryVoltage.unsafe_value();
    batteryLock->isValid = false;
    batteryLock->lastUpdate = 0;
    batteryLock->rawVoltage = 0;
    batteryLock->isCritical = false;
    BattCount = 1;
    BattTaskTime = DWT_SysTick_To_us() * DWT_GetTick();
}

void BatteryModule::entry(void) {
    battery.update();
    //printf("[INFO] Battery Voltage Percentage: %f\r\n", 100*battery.getBattPercentage());
    if (BattCount % 5==0) {
        printf("Battery Time elapsed: %d\r\n", BattTaskTime - DWT_SysTick_To_us() * DWT_GetTick());
        BattTaskTime = DWT_SysTick_To_us() * DWT_GetTick();
    }
    auto batteryLock = batteryVoltage.lock();
    batteryLock->isValid = true;
    batteryLock->lastUpdate = HAL_GetTick();
    batteryLock->rawVoltage = battery.getRaw();
    batteryLock->isCritical = battery.isBattCritical();
}
