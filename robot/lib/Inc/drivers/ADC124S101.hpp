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
    uint16_t read_register(uint8_t address);

    void chip_select(bool cs_state);

    LockedStruct<SPI>& adcSPI;

    DigitalOut nCs;
};
