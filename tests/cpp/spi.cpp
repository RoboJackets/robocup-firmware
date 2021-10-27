#include "SPI.hpp"
#include "mtrain.hpp"
#include <vector>

int main(void) {
    {
        SPI spi2(SpiBus5, p26, 2'000'000);

        spi2.transmit({1, 2, 3});

        for (int i = 0; i <= 100; i++) {
            spi2.transmitReceive(i);
        }
        
        spi2.frequency(8'000'000);

        std::vector<uint8_t> nums;
        for (int i = 1; i <= 100; i++) {
            nums.push_back(i);
        }
        spi2.transmitReceive(nums);

        spi2.frequency(1);
        for (uint8_t i = 0; i <= 100; i++) {
            spi2.transmit(i);
        }

        spi2.frequency(15'000'000);
        spi2.transmit(nums);
    }

    while (true) { }
}
