#include "modules/NewRadioModule.hpp"

#include <memory>

#include "mtrain.hpp"

using namespace std::literals;

NewRadioModule::NewRadioModule(LockedStruct<BatteryVoltage>& batteryVoltage)
    : GenericModule(kPeriod, "newRadio", kPriority),
      uart(UARTBus7),
      batteryVoltage(batteryVoltage) {
    // It makes no sense to actually attempt to lock the mutex here, because
    // the scheduler hasn't started up yet. There's also no chance of preemption
    // while we're using it, because we're in startup, so just use the struct.
}

void NewRadioModule::start() {}

void NewRadioModule::entry(void) {
    printf("[INFO] This is New Radio Module");
    uart.transmit(0x01);
    uart.receive(&dataBuff, sizeof(0x01));
    printf("[INFO] Value of data buffer is: %d", dataBuff);
}
