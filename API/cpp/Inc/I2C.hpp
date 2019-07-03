#pragma once

#include "PinDefs.hpp"

#include <vector>

typedef enum I2CBus {
    I2CBus1,
    I2CBus4,
} I2CBus;

class I2C {
public:
    I2C(I2CBus i2cBus);
    ~I2C();

    void transmit(uint8_t address, uint8_t regAddr, uint8_t data);
    void transmit(uint8_t address, uint8_t regAddr, const std::vector<uint8_t>& data);

    uint8_t receive(uint8_t address, uint8_t regAddr);
    std::vector<uint8_t> receive(uint8_t address, uint8_t regAddr, size_t count);

    void recover_bus();

private:

    I2C_HandleTypeDef i2cHandle = {};
};