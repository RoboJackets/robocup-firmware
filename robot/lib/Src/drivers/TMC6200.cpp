#include "drivers/TMC6200.hpp"
#include <cmath>

#define WRITE_BIT 0x80

#define VERSION_VAL 0x10

namespace Registers {
    constexpr uint8_t GCONF = 0x00;

    enum GConf {
        DISABLE = 0b1;
        SINGLELINE = 0b1 << 1;
        FAULTDIRECT = 0b1 << 2;
        AMPLIFICATION_5 = 0b00 << 4;
        AMPLIFICATION_10 = 0b01 << 4;
        AMPLIFICATION_15 = 0b10 << 4;
        AMPLIFICATION_20 = 0b11 << 4;
        AMPLIFIER_OFF = 0b1 << 6;
        TEST_MODE = 0b00 << 7;
    };

    constexpr uint8_t GSTAT = 0x01;

    enum GStat {
        RESET =  0b1;
        DRV_OTPW = 0b1 << 1;
        DRV_OT = 0b1 << 2;
        UV_CP = 0b1 << 3;
        SHORTDET_U = 0b1 << 4;
        S2GU = 0b1 << 5;
        S2VSU = 0b1 << 6;
        SHORTDET_V = 0b1 << 8;
        S2GV = 0b1 << 8;
        S2VSV = 0b1 << 10;
        SHORTDET_W = 0b1 << 12;
        S2GV = 0b1 << 13;
        S2VSV = 0b1 << 14;
        ERROR_BITMASK = 0b111011101111110
    };

    constexpr uint8_t IOIN = 0x04;

    enum IOIn {
        UL = 0b1;
        UH = 0b1 << 1;
        VL = 0b1 << 2;
        VH = 0b1 << 3;
        WL = 0b1 << 4;
        WH = 0b1 << 5;
        DRV_EN = 0b1 << 6;
        OTPW = 0b1 << 8;
        OT136C = 0b1 << 9;
        OT143C = 0b1 << 10;
        OT150C = 0b1 << 11;
        VERSION = 0b11111111 << 24;
    };

    constexpr uint8_t OTP_PROG = 0x06;

    enum OTP_Prog {
        OTPBYTE = 0x00;
        OTPMAGIC = 0xbd;
    };

    constexpr uint8_t OTP_READ = 0x07;
    constexpr uint8_t FACTORY_CONF = 0x08;
    constexpr uint8_t SHORT_CONF = 0x09;

    enum Short_Conf {
        S2VS_LEVEL_BITMASK = 0b1111;
        S2G_LEVEL_BITMASK = 0b1111 << 8;
        SHORTFILTER_100ns = 0b00 << 16;
        SHORTFILTER_1us = 0b01 << 16;
        SHORTFILTER_2us = 0b10 << 16;
        SHORTFILTER_3us = 0b11 << 16;
        SHORTDELAY_NORMAL = 0b0 << 20
        SHORTDELAY_HIGH = 0b1 << 20;
        RETRY_DISABLED = 0b00 << 24;
        RETRY_1 = 0b01 << 24;
        RETRY_2 = 0b10 << 24;
        RETRY_3 = 0b11 << 24;
        PROTECT_PARALLEL_PARTIAL = 0b0 << 28;
        PROTECT_PARALLEL_ALL = 0b1 << 28;
        ENABLE_S2G = 0b0 << 29;
        DISABLE_S2G = 0b1 << 29;
        ENABLE_S2VS = 0b0 << 30;
        DISABLE_S2VS = 0b1 << 30;
    };

    constexpr uint8_t DRV_CONF = 0x0A;

    enum DRV_Conf {
        BBMCLKS_BITMASK = 0b1111;
        OTSELECT_150C = 0b00 << 16;
        OTSELECT_143C = 0b01 << 16;
        OTSELECT_136C = 0b10 << 16;
        OTSELECT_120C = 0b11 << 16;
        DRV_STRENGTH_WEAK = 0b00 << 16;
        DRV_STRENGTH_WEAK_TC = 0b01 << 16;
        DRV_STRENGTH_MEDIUM = 0b10 << 16;
        DRV_STRENGTH_STRONG = 0b11 << 16;
    };

} // namespace Registers


TMC6200::TMC6200(LockedStruct<SPI>& tmcSPI, PinName cs_pin)
        : tmcSPI(tmcSPI),
          nCs(cs_pin) {
    nCs = 1;
}

bool TMC6200::initialize() {
    errors.fill(false);
    _has_Error = false;
    _temperature_Error = false;
    _phase_U_Short = false;
    _phase_V_Short = false;
    _phase_W_Short = false;

    printf("Connecting to TMC6200.");

    // TODO: Find out version value, then uncomment
    // uint8_t version = (read_register(Registers::IOIN) & Registers::IOIn::VERSION);
    //
    // while (version != VERSION_VAL) {
    //     printf("%d\n", version);
    //     printf("Failed to connect to TMC6200.");
    //     vTaskDelay(100);
    //     version = read_register(Registers::IOIN) & Registers::IOIn::VERSION;
    }

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
    uint32_t gstatData = read_register(TMC6200::GSTAT);
    // Check if any flags are set
    if (gStat & TMC6200::GStat::ERROR_BITMASK != 0x0) {
        for(int i = 0; i < bitMasks.size(); i++) {
            // If flag set, toggle error at corresponding index
            if ((gstatData & bitMasks[i]) != 0x0) {
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