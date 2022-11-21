#include "drivers/PCA9848.hpp"

PCA9848::PCA9848(LockedStruct<I2C>& sharedI2C, int i2cAddress)
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {}
void PCA9848::writeRegister(uint16_t data) {
    auto i2c_lock = _i2c.lock();
    uint8_t buffer=data;
    i2c_lock->transmit(0xE2h, buffer);
}