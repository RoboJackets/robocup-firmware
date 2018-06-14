#pragma once

#include "Assert.hpp"
#include "I2CDriver.hpp"
#include "Mbed.hpp"
#include "Rtos.hpp"

#include <memory>

class SharedI2C : public I2CDriver {
public:
    SharedI2C(PinName sda, PinName scl, int freq) : I2CDriver(sda, scl, freq) {
        ++m_objectCount;
        ASSERT(m_objectCount == 1);
    }

private:
    static uint8_t m_objectCount;
};

class SharedI2CDevice {
public:
    using I2CPtrT = std::shared_ptr<SharedI2C>;

    SharedI2CDevice(I2CPtrT i2c) : m_i2c(i2c) { ASSERT(i2c != nullptr); }

protected:
    I2CPtrT m_i2c;
};
