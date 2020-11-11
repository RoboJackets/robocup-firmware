#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include "LockedStruct.hpp"
#include <memory>
#include <vector>

class TMC4671 {
public:
    TMC4671(LockedStruct<SPI>& tmcSPI, PinName cs_pin);

    bool initialize();

private:
    void write_register(uint8_t bank, uint8_t address, uint8_t value);

    uint8_t read_register(uint8_t bank, uint8_t address);

    void chip_select(bool cs_state);

    LockedStruct<SPI>& tmcSPI;

    DigitalOut nCs;

    int last_bank = 0;
};
