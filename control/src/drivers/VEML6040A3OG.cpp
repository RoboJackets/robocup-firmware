#include "drivers/VEML6040A3OG.hpp"

VEML6040A3OG::VEML6040A3OG(LockedStruct<I2C>& sharedI2C, int i2cAddress)
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {
}