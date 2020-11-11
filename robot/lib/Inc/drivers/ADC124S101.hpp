#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include "LockedStruct.hpp"
#include <memory>
#include <vector>

class ADC124S101 {
public:
    ADC124S101(LockedStruct<SPI>& adcSPI, PinName cs_pin);

    bool initialize();

private:
    void write_register(uint8_t bank, uint8_t address, uint8_t value);

    uint8_t read_register(uint8_t bank, uint8_t address);

    void chip_select(bool cs_state);

    LockedStruct<SPI>& adcSPI;

    DigitalOut nCs;

    int last_bank = 0;
};
