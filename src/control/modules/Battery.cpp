#include "Battery.hpp"

Battery* Battery::globBatt = nullptr;

Battery::Battery() : battVoltageAIPin(RJ_BATT_SENSE) {}

Battery::~Battery() {}

/**
 * updates internal float and raw reads
 *
 * call once per iteration
 */
void Battery::update() {
    float voltage = (battVoltageAIPin.read() * 3.3f);
    lastReadPercentage =
        ((voltage - MIN_SAFE_BATT_VOLTAGE_READ) / BATT_VOLTAGE_READ_RANGE);
    rawVoltage = (battVoltageAIPin.read_u16() >> 8);
}

/**
 * gets battery percentage from the last update
 */
float Battery::getBattPercentage() { return lastReadPercentage; }

/**
 * get the raw read from the last update
 */
uint8_t Battery::getRaw() { return rawVoltage; }

/**
 * returns if the battery level is critical
 *
 * functionality should be reduced on critical voltage.
 * this includes motors.
 */
bool Battery::isBattCritical() { return lastReadPercentage <= 0.0; }
