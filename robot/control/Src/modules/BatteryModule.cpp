#include "modules/BatteryModule.hpp"
#include "mtrain.h"

BatteryModule::BatteryModule(BatteryVoltage *const batteryVoltage)
    : batteryVoltage(batteryVoltage) {
    
    batteryVoltage->isValid = false;
    batteryVoltage->lastUpdate = 0;
    batteryVoltage->rawVoltage = 0;
    batteryVoltage->isCritical = false;
}

void BatteryModule::entry(void) {
    battery.update();

    batteryVoltage->isValid = true;
    batteryVoltage->lastUpdate = HAL_GetTick();
    batteryVoltage->rawVoltage = battery.getRaw();
    batteryVoltage->isCritical = battery.isBattCritical();
}