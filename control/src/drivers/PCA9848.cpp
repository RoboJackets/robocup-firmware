#include "drivers/PCA9848.hpp"

PCA9848::PCA9848(LockedStruct<I2C>& sharedI2C) : _i2c(sharedI2C) {}

void PCA9848::writeRegister(uint8_t data) {
    auto i2c_lock = _i2c.lock();
    i2c_lock->transmit(I2C_BUS_ADDRESS, data);
}