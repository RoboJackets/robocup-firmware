#pragma once
#include "GenericModule.hpp"
#include "common.hpp"

class BlinkModule : public GenericModule {
public:
    BlinkModule(std::chrono::milliseconds period = {100},
                PinName pin_name = LED1,
                const char* name = "blink")
        : GenericModule(period, name), pin(LED1) {}

    void entry() override {
        pin.write(value);
        value = !value;
    }

private:
    DigitalOut pin;
    bool value = false;
};
