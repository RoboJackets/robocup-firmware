#include "LockedStruct.hpp"
#include "drivers/PCA9858PWJ.hpp"

PCA9858PWJ::PCA9858PWJ(LockedStruct<I2C> &sharedI2C, int i2cAddress) : _i2c(sharedI2C), _i2cAddress(i2cAddress)
{

}

// write to a specific register
void PCA9858PWJ::writeRegister(PCA9858PWJ::Register regAddress, uint16_t data)
{
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(data & 0xff), // Essentially, splits the data into the lower 8 bits
                                static_cast<uint8_t>(data >> 8)};  // and the upper 8 bits (the first and second index respectively)
    i2c_lock->transmit(_i2cAddress, regAddress, buffer);
}

// reads a specific register
void PCA9858PWJ::readRegister(PCA9858PWJ::Register regAddress)
{
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer = i2c_lock->receive(_i2cAddress, regAddress, 2);
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

// TODO: look here pls
// https://github.com/torvalds/linux/blob/master/drivers/i2c/muxes/i2c-mux-pca954x.c
