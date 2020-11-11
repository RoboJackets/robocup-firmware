#include "mtrain.hpp"
#include "drivers/TMC6200.hpp"

int main() {
    LockedStruct<SPI> spi(SpiBus::SpiBus1, std::nullopt, 100'000);
    TMC6200 tmc{spi, p7};

    tmc.initialize();

    printf("I didn't crash!");
}
