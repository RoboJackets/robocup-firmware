#pragma once

#include <memory>
#include "I2C.hpp"
#include "LockedStruct.hpp"

class VEML6040A3OG {
    private:
    LockedStruct<I2C>& _i2c;
    int _i2cAddress;  // physical I2C address

    // Cached copies of the register values
    uint16_t _cachedGPIO, _cachedIODIR, _cachedGPPU, _cachedIPOL;
};