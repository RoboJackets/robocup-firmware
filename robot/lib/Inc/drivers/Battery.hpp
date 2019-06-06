#pragma once

#include "cstdint"

class Battery {
public:
    Battery();

    void update();

    float getBattPercentage();

    uint8_t getRaw();

    bool isBattCritical();

private:
    // 5-cell lipo * 4.3 v per cell = 21.5 battery volts
    // 68k and 10k ohm voltage divider, analog in read votlage 2.756
    const float MAX_SAFE_BATT_VOLTAGE_READ = 2.756;

    // 5-cell lipo * 3.0 v per cell = 15.0 battery volts
    // 68k and 10k ohm voltage divider, analog in read votlage 1.923
    const float MIN_SAFE_BATT_VOLTAGE_READ = 1.923;

    const float BATT_VOLTAGE_READ_RANGE =
        (MAX_SAFE_BATT_VOLTAGE_READ - MIN_SAFE_BATT_VOLTAGE_READ);

    //AnalogIn battVoltageAIPin;

    float lastReadPercentage;
    uint8_t rawVoltage;
};