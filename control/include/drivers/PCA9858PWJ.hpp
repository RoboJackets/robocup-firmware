#include "I2C.hpp"
//#define CONTROL 0xE2

class PCA9858PWJ
{
public:
    uint8_t readRegister();
    void writeRegister(uint8_t data);
    PCA9858PWJ(I2C &sharedI2C);

private:
    I2C &_i2c;
};