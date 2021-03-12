#include "drivers/TMC6200.hpp"
#include <cmath>

#define WRITE_BIT 0x80

#define VERSION_VAL 0x10


TMC6200::TMC6200(LockedStruct<SPI>& tmcSPI, PinName cs_pin)
        : tmcSPI(tmcSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool TMC6200::initialize() {
    errors.fill(false);

    // TODO: Find out version value, then uncomment
    // uint8_t version = (read_register(Registers::IOIN) & Registers::IOIn::VERSION);
    //
    // while (version != VERSION_VAL) {
    //     printf("%d\n", version);
    //     printf("Failed to connect to TMC6200.");
    //     vTaskDelay(100);
    //     version = read_register(Registers::IOIN) & Registers::IOIn::VERSION;

    printf("Connected to TMC6200.");

    return true;
}

void TMC6200::chip_select(bool cs_state) {
    nCs.write(!cs_state);
}

void TMC6200::write_register(uint8_t address, uint32_t value) {
    auto lock = tmcSPI.lock();
    chip_select(true);
    // SPI Datagram
    // Bits 39...32: MSB (RW) + 7-bit address
    // Bits 31...00: 32-bit data
    lock->transmit(((WRITE_BIT | address) << 32) | value);
    chip_select(false);

}

uint32_t TMC6200::read_register(uint8_t address) {
    auto lock = tmcSPI.lock();

    chip_select(true);
    uint32_t data = lock->transmitReceive(address);
    chip_select(false);

    return data;
}

void TMC6200::checkForErrors() {
    uint32_t gStatData = read_register(Registers::GSTAT);
    // Check if any flags are set
    if (gStatData & Registers::GStat::ERROR_BITMASK != 0x0) {
        for(int i = 0; i < bitMasks.size(); i++) {
            // If flag set, toggle error at corresponding index
            if ((gStatData & bitMasks[i]) != 0x0) {
                errors[i] = true;
            }
        }
    }
}

bool TMC6200::hasTemperatureError() {
    return errors[0] || errors[1];
}

bool TMC6200::hasPhaseUShort() {
    return errors[3] || errors[4] || errors[5];
}

bool TMC6200::hasPhaseVShort() {
    return errors[6] || errors[7] || errors[8];
}

bool TMC6200::hasPhaseWShort() {
    return errors[9] || errors[10] || errors[11];
}

bool TMC6200::hasError() {
    return hasTemperatureError() || hasPhaseUShort() || hasPhaseVShort() || hasPhaseWShort();
}