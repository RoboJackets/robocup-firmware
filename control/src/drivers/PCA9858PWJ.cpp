#include "LockedStruct.hpp"
#include "drivers/PCA9858PWJ.hpp"
#include "I2C.hpp"

PCA9858PWJ::PCA9858PWJ(I2C &sharedI2C)
    : _i2c(sharedI2C)
{
}

// write to a specific register
void PCA9858PWJ::writeRegister(uint8_t data)
{
    // auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(data & 0xff), // Essentially, splits the data into the lower 8 bits
                                static_cast<uint8_t>(data >> 8)};  // and the upper 8 bits (the first and second index respectively)
    _i2c.transmit(0xE2, 0, data);
}

// reads a specific register
uint16_t PCA9858PWJ::readRegister()
{
    // auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer = _i2c.receive(0xE2, 0, 2);
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

// TODO: look here pls
// https://github.com/torvalds/linux/blob/master/drivers/i2c/muxes/i2c-mux-pca954x.c
