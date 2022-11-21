#pragma once

#include <memory>
#include "I2C.hpp"
#include "LockedStruct.hpp"

class PCA9848 {
    public:
    PCA9848(LockedStruct<I2C>& sharedI2C, int i2cAddress);
    private:
    LockedStruct<I2C>& _i2c;
    int _i2cAddress;  // physical I2C address
    void PCA9848::writeRegister(uint16_t data);
    

    // Cached copies of the register values
    //uint16_t _cachedGPIO, _cachedIODIR, _cachedGPPU, _cachedIPOL;
};