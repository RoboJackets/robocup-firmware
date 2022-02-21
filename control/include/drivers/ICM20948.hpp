#pragma once

#include "robocup.hpp"
#include <memory>
#include <vector>
#include <cmath>

class ICM20948 {
public:
    ICM20948(LockedStruct<SPI>& imuSPI, PinName cs_pin);

    bool initialize();

    double gyro_z();

    double accel_x();

    double accel_y();

    void burst_read(uint8_t bank, uint8_t address, uint8_t* buffer_out, size_t length);

private:
    void write_register(uint8_t bank, uint8_t address, uint8_t value);

    uint8_t read_register(uint8_t bank, uint8_t address);

    void chip_select(bool cs_state);

    LockedStruct<SPI>& imuSPI;

    DigitalOut nCs;

    int last_bank = 0;
};
