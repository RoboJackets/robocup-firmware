#include "mtrain.hpp"
#include "drivers/ADC124S101.hpp"

int main() {
    LockedStruct<SPI> spi(SpiBus::SpiBus2, std::nullopt, 100'000);
    ADC124S101 adc{spi, p31};

    adc.initialize();
}
