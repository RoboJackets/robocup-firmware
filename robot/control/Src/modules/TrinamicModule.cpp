#include "modules/TrinamicModule.hpp"

TrinamicModule::TrinamicModule(LockedStruct<SPI>& spi, LockedStruct<TrinamicInfo>& trinamicInfo):
                        kiGenericModule(kPeriod, "trinamic", kPriority), tmc6200(spi), trinamicInfo(trinamicInfo){
    auto trinamicInfoLock = trinamicInfo.unsafe_value();
    trinamicInfoLock->isValid = false;
    trinamicInfoLock->lastUpdate = 0;
    trinamicInfoLock->trinamicHasError = false;
}

void TrinamicModule::start() {
    trinamicInfoLock->initialized = true;
    printf("INFO: Trinamic Boards initialized\r\n");
}

void TrinamicModule::entry() {
    tmc6200.checkForErrors();
    {
        auto trinamicInfoLock = trinamicInfo.lock();
        trinamicInfoLock->isValid = true;
        trinamicInfoLock->lastUpdate = HAL_GetTick();
        trinamicInfoLock->trinamicHasError = tmc6200.hasError();
        trinamicInfoLock->temperatureError = tmc6200.hasTemperatureError();
        trinamicInfoLock->phaseUShort = tmc6200.hasPhaseUShort();
        trinamicInfoLock->phaseVShort = tmc6200.hasPhaseVShort();
        trinamicInfoLock->phaseWShort = tmc6200.hasPhaseWShort();

    }
}