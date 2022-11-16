#include "I2C.hpp"
#include "mtrain.hpp"

/* I2C test using w/ blocking transmit and receive functions */
int main() {
    I2C i2c1(I2CBus1);
    fflush(stdout);
    while (true) {
        i2c1.transmit(0xE2, 0b00000010);
    }
}