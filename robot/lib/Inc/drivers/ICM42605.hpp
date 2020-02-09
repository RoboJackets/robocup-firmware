#pragma once

#include <memory>
#include "SPI.hpp"
#include "DigitalOut.hpp"

static constexpr uint8_t ICM42605_ADDRESS = 0x69;

class ICM42605 {
public:
    ICM42605(std::shared_ptr<SPI> spi_bus, PinName cs_pin);

    void initialize();

    // Returns gyro speed in radians per second.
    float gyro_z();

private:
    void chip_select(bool active);
    uint8_t read_register(uint8_t address);
    void write_register(uint8_t address, uint8_t value);

    std::shared_ptr<SPI> spi_bus;
    DigitalOut chip_select_pin;

    bool currently_active = false;
};
