#include "drivers/PCA9848.hpp"
#include "LockedStruct.hpp"

PCA9848::PCA9848(LockedStruct<I2C>& sharedI2C, int i2cAddress)
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {
    }

void PCA9848::writeRegister(PCA9848::Register regAddress, uint16_t data) {
    auto i2c_lock = _i2c.lock();
    i2c_lock->transmit(_i2cAddress, regAddress, buffer);
}