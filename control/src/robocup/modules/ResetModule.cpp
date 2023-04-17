#include "modules/ResetModule.hpp"

ResetModule::ResetModule()
    : GenericModule(kPeriod, "reset", kPriority) {
}

void ResetModule::start() {

}

void ResetModule::entry(void) {
    // :)
    NVIC_SystemReset();
}
