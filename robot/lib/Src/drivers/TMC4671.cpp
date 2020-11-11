#include "drivers/TMC4671.hpp"
#include <cmath>

#define WRITE_BIT 0x80

#define CHIPINFO_VAL 0x34363731

namespace Registers {
    constexpr uint8_t CHIPINF = 0x04;
} // namespace Registers


TMC4671::TMC4671(LockedStruct<SPI>& tmcSPI, PinName cs_pin);
        : tmcSPI(tmcSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool TMC4671::initialize() {
    uint32_t chipinfo = (read_register(Registers::VERSION) >> 24);

    while (chipinfo != VERSION_VAL) {
        printf(chipinfo)
        printf("Failed to connect to TMC4671.");
        vTaskDelay(100);
        chipinfo = read_register(Registers::VERSION);
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
