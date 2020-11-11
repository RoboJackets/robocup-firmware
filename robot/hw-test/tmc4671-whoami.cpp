#include "mtrain.hpp"
#include "drivers/TMC4671.hpp"

int main() {
    LockedStruct<SPI> spi(SpiBus::SpiBus1, std::nullopt, 100'000);
    TMC4671 tmc{spi, p8};

    tmc.initialize();

    printf("I didn't crash!");
}
