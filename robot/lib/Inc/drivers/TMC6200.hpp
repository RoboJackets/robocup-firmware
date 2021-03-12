#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include "LockedStruct.hpp"
#include <array>
#include <memory>
#include <vector>

namespace Registers {
    constexpr uint8_t GCONF = 0x00;

    enum GConf {
        DISABLE = 0b1,
        SINGLELINE = 0b1 << 1,
        FAULTDIRECT = 0b1 << 2,
        AMPLIFICATION_5 = 0b00 << 4,
        AMPLIFICATION_10 = 0b01 << 4,
        AMPLIFICATION_15 = 0b10 << 4,
        AMPLIFICATION_20 = 0b11 << 4,
        AMPLIFIER_OFF = 0b1 << 6,
        TEST_MODE = 0b00 << 7,
    };

    constexpr uint8_t GSTAT = 0x01;

    enum GStat {
        RESET =  0b1,
        DRV_OTPW = 0b1 << 1,
        DRV_OT = 0b1 << 2,
        UV_CP = 0b1 << 3,
        SHORTDET_U = 0b1 << 4,
        S2GU = 0b1 << 5,
        S2VSU = 0b1 << 6,
        SHORTDET_V = 0b1 << 8,
        S2GV = 0b1 << 8,
        S2VSV = 0b1 << 10,
        SHORTDET_W = 0b1 << 12,
        S2GW = 0b1 << 13,
        S2VSW = 0b1 << 14,
        ERROR_BITMASK = 0b111011101111110,
    };

    constexpr uint8_t IOIN = 0x04;

    enum IOIn {
        UL = 0b1,
        UH = 0b1 << 1,
        VL = 0b1 << 2,
        VH = 0b1 << 3,
        WL = 0b1 << 4,
        WH = 0b1 << 5,
        DRV_EN = 0b1 << 6,
        OTPW = 0b1 << 8,
        OT136C = 0b1 << 9,
        OT143C = 0b1 << 10,
        OT150C = 0b1 << 11,
        VERSION = 0b11111111 << 24,
    };

    constexpr uint8_t OTP_PROG = 0x06;

    enum OTP_Prog {
        OTPBYTE = 0x00,
        OTPMAGIC = 0xbd,
    };

    constexpr uint8_t OTP_READ = 0x07;
    constexpr uint8_t FACTORY_CONF = 0x08;
    constexpr uint8_t SHORT_CONF = 0x09;

    enum Short_Conf {
        S2VS_LEVEL_BITMASK = 0b1111,
        S2G_LEVEL_BITMASK = 0b1111 << 8,
        SHORTFILTER_100ns = 0b00 << 16,
        SHORTFILTER_1us = 0b01 << 16,
        SHORTFILTER_2us = 0b10 << 16,
        SHORTFILTER_3us = 0b11 << 16,
        SHORTDELAY_NORMAL = 0b0 << 20,
        SHORTDELAY_HIGH = 0b1 << 20,
        RETRY_DISABLED = 0b00 << 24,
        RETRY_1 = 0b01 << 24,
        RETRY_2 = 0b10 << 24,
        RETRY_3 = 0b11 << 24,
        PROTECT_PARALLEL_PARTIAL = 0b0 << 28,
        PROTECT_PARALLEL_ALL = 0b1 << 28,
        ENABLE_S2G = 0b0 << 29,
        DISABLE_S2G = 0b1 << 29,
        ENABLE_S2VS = 0b0 << 30,
        DISABLE_S2VS = 0b1 << 30,
    };

    constexpr uint8_t DRV_CONF = 0x0A;

    enum DRV_Conf {
        BBMCLKS_BITMASK = 0b1111,
        OTSELECT_150C = 0b00 << 16,
        OTSELECT_143C = 0b01 << 16,
        OTSELECT_136C = 0b10 << 16,
        OTSELECT_120C = 0b11 << 16,
        DRV_STRENGTH_WEAK = 0b00 << 16,
        DRV_STRENGTH_WEAK_TC = 0b01 << 16,
        DRV_STRENGTH_MEDIUM = 0b10 << 16,
        DRV_STRENGTH_STRONG = 0b11 << 16,
    };

} // namespace Registers

class TMC6200 {
public:
    TMC6200(LockedStruct<SPI>& tmcSPI, PinName cs_pin);

    bool initialize();
    bool hasTemperatureError();
    bool hasPhaseUShort();
    bool hasPhaseVShort();
    bool hasPhaseWShort();
    bool hasError();

    void checkForErrors();

    uint32_t read_register(uint8_t address);

private:
    void write_register(uint8_t address, uint32_t value);


    void chip_select(bool cs_state);

    LockedStruct<SPI>& tmcSPI;

    DigitalOut nCs;

    /**
     * Encodes state of TMC6200 Status Flags
     * Refer to TMC6200 datasheet
     * Index | Name         | Meaning
     * 0     | drv_optw     | Overtemperature prewarning level exceeded
     * 1     | drv_ot       | Driver shut down due to overtemperature
     * 2     | uv_cp        | Under-voltage on charge pump
     * 3     | shortdet_u   | U short counter triggered at least once
     * 4     | s2gu         | Short to GND detected on phase U
     * 5     | s2vsu        | Short to VS detected on phase U
     * 6     | shortdet_v   | V short counter triggered at least once
     * 7     | s2gv         | Short to GND detected on phase V
     * 8     | s2vsv        | Short to VS detected on phase V
     * 9     | shortdet_w   | W short counter triggered at least once
     * 10    | s2gw         | Short to GND detected on phase W
     * 11    | s2vsw        | Short to VS detected on phase W
     */
    std::array<bool, 12> errors;
    const std::array<uint32_t, 12> bitMasks = {Registers::GStat::DRV_OTPW,
                                                 Registers::GStat::DRV_OT,
                                                 Registers::GStat::UV_CP,
                                                 Registers::GStat::SHORTDET_U,
                                                 Registers::GStat::S2GU,
                                                 Registers::GStat::S2VSU,
                                                 Registers::GStat::SHORTDET_V,
                                                 Registers::GStat::S2GV,
                                                 Registers::GStat::S2VSV,
                                                 Registers::GStat::SHORTDET_W,
                                                 Registers::GStat::S2GW,
                                                 Registers::GStat::S2VSW};
};
