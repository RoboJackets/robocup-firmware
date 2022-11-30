#include "drivers/VEML6040.hpp"

#include <cmath>

#include "LockedStruct.hpp"

VEML6040::VEML6040(LockedStruct<I2C>& sharedI2C, int i2cAddress)
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {}

void VEML6040::init() {
    auto i2c_lock = _i2c.lock();
    // it = 011
    // trig = 0
    // af = 0
    // sd = 0
    i2c_lock->transmit(_i2cAddress, CONF, 0b00110000);
}

/*
void VEML6040::writeRegister(VEML6040::Register commandCode, uint16_t data) {
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(commandCode),
                                static_cast<uint8_t>(data & 0xff), static_cast<uint8_t>(data >> 8)};
    i2c_lock->transmit(_i2cAddress, buffer);
}*/

uint16_t VEML6040::readRegister(VEML6040::Register regAddress) {
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer = i2c_lock->receive(_i2cAddress, regAddress, 2);
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

int VEML6040::getR() {
    uint16_t data = readRegister(R_DATA);
    return round(data * GSensitivity / MaxLux * 255);
}

int VEML6040::getG() {
    uint16_t data = readRegister(G_DATA);
    return round(data * GSensitivity / MaxLux * 255);
}

int VEML6040::getB() {
    uint16_t data = readRegister(B_DATA);
    return round(data * GSensitivity / MaxLux * 255);
}

int VEML6040::getW() {
    uint16_t data = readRegister(W_DATA);
    return round(data * GSensitivity / MaxLux * 255);
}