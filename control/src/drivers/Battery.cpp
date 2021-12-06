#include "drivers/Battery.hpp"

Battery::Battery()
    : lastReadPercentage(0.0f), rawVoltage(0), battVoltagePin(adc) {
}

void Battery::update() {
    lastReadPercentage = (battVoltagePin.getVoltage() - MIN_SAFE_BATT_VOLTAGE_READ) / BATT_VOLTAGE_READ_RANGE;
    rawVoltage = (uint8_t)(lastReadPercentage * 255);
}

float Battery::getBattPercentage() {
    return lastReadPercentage;
}

uint8_t Battery::getRaw() {
    return rawVoltage;
}

bool Battery::isBattCritical() {
    this->update();
    float voltageValue = this->getVoltage(); 
    return voltageValue < 3.3 || voltageValue > 5

 //  return lastReadPercentage <= 0.05 || lastReadPercentage > 1.0;
}

float Battery::getVoltage() {
    return battVoltagePin.getVoltage();
}
