#include "drivers/ADC124S101.hpp"
#include <cmath>

namespace Registers {
    constexpr uint8_t CHANNEL1 = 0x00;
    constexpr uint8_t CHANNEL2 = 0x08;
    constexpr uint8_t CHANNEL3 = 0x10;
    constexpr uint8_t CHANNEL4 = 0x18;
} // namespace Registers


ADC124S101::ADC124S101(LockedStruct<SPI>& adcSPI, PinName cs_pin)
        : adcSPI(adcSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool ADC124S101::initialize() {
    printf("Connecting to ADC124S101.");

    uint16_t channel4 = read_register(Registers::CHANNEL4);

    while (true) {
        printf("%d\n", channel4);
        vTaskDelay(100);
        channel4 = read_register(Registers::CHANNEL4);
    }

    return true;
}

void ADC124S101::chip_select(bool cs_state) {
    nCs.write(!cs_state);
}

uint16_t ADC124S101::read_register(uint8_t address) {
    auto lock = adcSPI.lock();

    chip_select(true);
    uint16_t data = lock->transmitReceive(address);
    chip_select(false);

    return data;
}
