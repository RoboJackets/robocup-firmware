#include "drivers/TMC6200.hpp"
#include <cmath>

#define WRITE_BIT 0x80

#define VERSION_VAL 0x10

namespace Registers {
    constexpr uint8_t VERSION = 0x04;
} // namespace Registers


TMC6200::TMC6200(LockedStruct<SPI>& tmcSPI, PinName cs_pin);
        : tmcSPI(tmcSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool TMC6200::initialize() {
    uint32_t version = (read_register(Registers::VERSION) >> 24);

    while (version != VERSION_VAL) {
        printf(version)
        printf("Failed to connect to TMC6200.");
        vTaskDelay(100);
        version = read_register(Registers::VERSION);
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
