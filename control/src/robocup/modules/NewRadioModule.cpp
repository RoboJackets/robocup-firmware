#include "modules/NewRadioModule.hpp"
#include "mtrain.hpp"
#include <memory>

using namespace std::literals;

NewRadioModule::NewRadioModule()
    : GenericModule(1000ms, "uart") {

    // It makes no sense to actually attempt to lock the mutex here, because
    // the scheduler hasn't started up yet. There's also no chance of preemption
    // while we're using it, because we're in startup, so just use the struct.
}

void NewRadioModule::entry(void) {
    printf("[INFO] This is New Radio Module");
}
