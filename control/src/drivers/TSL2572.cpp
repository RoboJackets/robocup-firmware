#include "LockedStruct.hpp"
#include "drivers/TSL2572.hpp"

TSL2572::TSL2572(LockedStruct<I2C>& sharedI2C, int i2cAddress)
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {
}

// write to a specific register
void TSL2572::writeRegister(TSL2572::Register regAddress, uint16_t data) {
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(data & 0xff),
                                static_cast<uint8_t>(data >> 8)};
    i2c_lock->transmit(_i2cAddress, regAddress, buffer);
}

// TODO: write the function that reads a specific register
void TSL2572::readRegister(TSL2572::Register regAddress) {
	auto i2c_lock = _i2c.lock(); 
	std::vector<uint8_t> buffer = i2c_lock->receive(_i2cAddress, regAddress, 2);
	return (uint16_t)(buffer[0] | (buffer[1] << 8));

}


