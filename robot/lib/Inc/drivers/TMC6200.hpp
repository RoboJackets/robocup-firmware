#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include "LockedStruct.hpp"
#include <array>
#include <memory>
#include <vector>

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

private:
    void write_register(uint8_t address, uint32_t value);

    uint32_t read_register(uint8_t address);

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
    const std::array<TMC6200::GStat, 12> bitMasks = {TMC6200::GStat::DRV_OTPW,
                                                     TMC6200::GStat::DRV_OT,
                                                     TMC6200::GStat::UV_CP,
                                                     TMC6200::GStat::SHORTDET_U,
                                                     TMC6200::GStat::S2GU,
                                                     TMC6200::GStat::S2VSU,
                                                     TMC6200::GStat::SHORTDET_V,
                                                     TMC6200::GStat::S2GV,
                                                     TMC6200::GStat::S2VSV,
                                                     TMC6200::GStat::SHORTDET_W,
                                                     TMC6200::GStat::S2GW,
                                                     TMC6200::GStat::S2VSW};
};
