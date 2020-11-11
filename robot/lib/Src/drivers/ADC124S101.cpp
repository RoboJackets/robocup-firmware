#include "drivers/ADC124S101.hpp"
#include <cmath>

namespace Registers {
    constexpr uint8_t CHANNEL1 = 0x00;
    constexpr uint8_t CHANNEL2 = 0x08;
    constexpr uint8_t CHANNEL3 = 0x10;
    constexpr uint8_t CHANNEL4 = 0x18;
} // namespace Registers


ADC124S101::ADC124S101(LockedStruct<SPI>& adcSPI, PinName cs_pin);
        : adcSPI(adcSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool TMC6200::initialize() {
    uint16_t channel4 = read_register(Registers::CHANNEL4);

    while (TRUE) {
        printf(channel4)
        vTaskDelay(100);
        channel4 = read_register(Registers::CHANNEL4);
    }
}

void TMC6200::chip_select(bool cs_state) {
    nCs.write(!cs_state);
}

void TMC6200::write_register(uint8_t address, uint32_t value) {
}

uint32_t TMC6200::read_register(uint8_t address) {
    auto lock = imuSPI.lock();

    chip_select(true);
    uint32_t data = lock->transmitReceive(address);
    chip_select(false);

    return data;
}
