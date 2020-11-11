#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include "LockedStruct.hpp"
#include <memory>
#include <vector>

class TMC6200 {
public:
    TMC6200(LockedStruct<SPI>& tmcSPI, PinName cs_pin);

    bool initialize();

private:
    void write_register(uint8_t address, uint32_t value);

    uint32_t read_register(uint8_t address);

    void chip_select(bool cs_state);

    LockedStruct<SPI>& tmcSPI;

    DigitalOut nCs;
};
