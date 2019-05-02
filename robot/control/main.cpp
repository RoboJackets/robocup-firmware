#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>

#include "drivers/ISM43340.hpp"

int main() {

    SPI radioSPI(SpiBus5, p17, 6'000'000);
    ISM43340 radioDriver(radioSPI, p17, p19, p18);

    bool test = radioDriver.selfTest();
    // bool test = true;

    DigitalOut l1 = DigitalOut(LED1);
    DigitalOut l2 = DigitalOut(LED2);
    l1 = 1;
    l2 = test;

  while (true) { }
  /*

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
  */
}
