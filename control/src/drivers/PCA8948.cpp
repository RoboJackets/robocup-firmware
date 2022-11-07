#include "LockedStruct.hpp"
#include "drivers/PCA8948.hpp"

PCA8948::PCA8948(LockedStruct<I2C>& sharedI2C) 
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {
}

void writeRegister(PCA8948::Register regAddress, uint16_t val) {
    auto i2c_lock = _i2c.lock();
    //we dont know what regaddress is yet but he'll tell us later
    i2c_lock->transmit(_i2cAddress, regAddress, val);
}