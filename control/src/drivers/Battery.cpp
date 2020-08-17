#include "drivers/Battery.hpp"

Battery::Battery()
    : lastReadPercentage(0.0f), rawVoltage(0) {
    // init analog pin
}

void Battery::update() {
    // read analog pin
    lastReadPercentage = 100.0;
    rawVoltage = 255;
}

float Battery::getBattPercentage() {
    return lastReadPercentage;
}

uint8_t Battery::getRaw() {
    return rawVoltage;
}

bool Battery::isBattCritical() {
    return lastReadPercentage <= 0.0;
}