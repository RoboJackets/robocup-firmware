#include "drivers/KickerBoard.hpp"
#include <tuple>

using namespace std;

KickerBoard::KickerBoard(shared_ptr<SPI> spi, PinName nCs,
                         PinName nReset, PinName ball_led)
    : AVR910(spi, nCs, nReset), _spi(spi), _nCs(nCs) {}
      //ballSenseLED(ball_led) {} todo figure out which pin this is

bool KickerBoard::verify_param(const char* name, char expected,
                               int (AVR910::*paramMethod)(), char mask,
                               bool verbose) {
    if (verbose) printf("Checking %s...", name);
    int val = (*this.*paramMethod)();
    bool success = ((val & mask) == expected);
    if (verbose) {
        if (success)
            printf("done\r\n");
        else
            printf("Got unexpected value: 0x%X\r\n", val);
    }

    return success;
}

bool KickerBoard::flash(const string& proFilename,
                        bool onlyIfDifferent, bool verbose) {
    // Check a few parameters before attempting to flash to ensure that we have
    // the right chip and it's connected correctly.
    auto checks = {
        make_tuple("Vendor ID", ATMEL_VENDOR_CODE, &AVR910::readVendorCode,
                   0xFF),
        make_tuple("Part Family", AVR_FAMILY_ID,
                   &AVR910::readPartFamilyAndFlashSize, AVR_FAMILY_MASK),
        make_tuple("Device ID", ATTINY_DEVICE_ID, &AVR910::readPartNumber,
                   0xFF),
    };
    for (auto& check : checks) {
        if (!verify_param(get<0>(check), get<1>(check), get<2>(check),
                          get<3>(check), verbose)) {
            return false;
        }
    }

    //  Open binary file to write to AVR.
    FILE* fp = fopen(proFilename.c_str(), "r");

    if (fp == nullptr) {
        //LOG(WARN, "Failed to open kicker binary, check path: '%s'",
        //    proFilename.c_str());
        exitProgramming();
        return false;
    } else {
        // Program it!
        //LOG(INFO, "Opened kicker binary, attempting to program kicker.");
        bool shouldProgram = true;
        if (onlyIfDifferent &&
            (checkMemory(ATTINY_PAGESIZE, ATTINY_NUM_PAGES, fp, false) == 0))
            shouldProgram = false;

        if (!shouldProgram) {
            //LOG(INFO, "Kicker up-to-date, no need to flash.");

            // exit programming mode by bringing nReset high
            exitProgramming();
        } else {
            bool success = program(fp, ATTINY_PAGESIZE, ATTINY_NUM_PAGES);

            if (!success) {
                //LOG(WARN, "Failed to program kicker.");
            } else {
                //LOG(INFO, "Kicker successfully programmed.");
            }
        }

        fclose(fp);
    }

    return true;
}

bool KickerBoard::flash(bool onlyIfDifferent, bool verbose) {
    // Check a few parameters before attempting to flash to ensure that we have
    // the right chip and it's connected correctly.
    auto checks = {
        make_tuple("Vendor ID", ATMEL_VENDOR_CODE, &AVR910::readVendorCode,
                   0xFF),
        make_tuple("Part Family", AVR_FAMILY_ID,
                   &AVR910::readPartFamilyAndFlashSize, AVR_FAMILY_MASK),
        make_tuple("Device ID", ATTINY_DEVICE_ID, &AVR910::readPartNumber,
                   0xFF),
    };
    for (auto& check : checks) {
        if (!verify_param(get<0>(check), get<1>(check), get<2>(check),
                          get<3>(check), verbose)) {
            return false;
        }
    }

    uint8_t* progBinary = 0; // todo use script
    unsigned int length = 0;

    // Program it!
    printf("Attempting to program kicker.\r\n");
    bool shouldProgram = true;
    if (onlyIfDifferent &&
        (checkMemory(ATTINY_PAGESIZE, ATTINY_NUM_PAGES, progBinary, length, false) == 0))
        shouldProgram = false;

    if (!shouldProgram) {
        //LOG(INFO, "Kicker up-to-date, no need to flash.");
        printf("Kicker up-to-date, no need to flash.\r\n");

        // exit programming mode by bringing nReset high
        exitProgramming();
    } else {
        bool success = program(progBinary, length, ATTINY_PAGESIZE, ATTINY_NUM_PAGES);

        if (!success) {
            //LOG(WARN, "Failed to program kicker.");
            printf("Failed to program kicker.\r\n");
        } else {
            //LOG(INFO, "Kicker successfully programmed.");
            printf("Kicker successfully programmed.\r\n");
        }
    }

    return true;
}

void KickerBoard::service() {
    // function that actually executes commands given to kicker
    //wait_us(100);
    HAL_Delay(1); // todo replace with us wait

    _nCs = 0;
    if (_kick_type_commanded) {
        _kick_type_commanded = false;
        if (_is_chip) {
            send_to_kicker(KICK_TYPE_CMD, DO_CHIP, nullptr);
        } else {
            send_to_kicker(KICK_TYPE_CMD, DO_KICK, nullptr);
        }
        _is_chip = false;
    }

    if (_kick_immediate_commanded) {
        _kick_immediate_commanded = false;
        send_to_kicker(KICK_IMMEDIATE_CMD, _kick_strength, nullptr);
        _kick_strength = 0;
    }

    if (_kick_breakbeam_commanded) {
        _kick_breakbeam_commanded = false;

        send_to_kicker(KICK_BREAKBEAM_CMD, _kick_strength, nullptr);

        _kick_strength = 0;
    }

    if (_cancel_breakbeam_commanded) {
        _cancel_breakbeam_commanded = false;

        if (_is_breakbeam_armed) {
            send_to_kicker(KICK_BREAKBEAM_CANCEL_CMD, BLANK, nullptr);
        }
    }

    if (_charging_commanded) {
        _charging_commanded = false;

        if (!_is_charging) {
            send_to_kicker(SET_CHARGE_CMD, ON_ARG, nullptr);
        }
    }

    if (_stop_charging_commanded) {
        _stop_charging_commanded = false;

        if (_is_charging) {
            send_to_kicker(SET_CHARGE_CMD, OFF_ARG, nullptr);
        }
    }

    _is_healthy = send_to_kicker(GET_VOLTAGE_CMD, BLANK, &_current_voltage);

    //wait_us(100);
    HAL_Delay(1);
    _nCs = 1;
}

bool KickerBoard::send_to_kicker(uint8_t cmd, uint8_t arg, uint8_t* ret_val) {
    //LOG(DEBUG, "Sending: CMD:%02X, ARG:%02X", cmd, arg);
    //wait_us(100);
    HAL_Delay(1);
    _spi->transmit(cmd);
    //wait_us(100);
    HAL_Delay(1);
    uint8_t command_resp = _spi->transmitReceive(arg);
    //wait_us(600);
    HAL_Delay(1);
    uint8_t ret = _spi->transmitReceive(BLANK);
    //wait_us(600);
    HAL_Delay(1);
    uint8_t state = _spi->transmitReceive(BLANK);
    //wait_us(100);
    HAL_Delay(1);

    _is_charging = state & (1 << CHARGE_FIELD);
    _ball_sensed = state & (1 << BALL_SENSE_FIELD);
    _is_breakbeam_armed = state & (1 << KICK_ON_BREAKBEAM_FIELD);
    _is_kicking = state & (1 << KICKING_FIELD);

    if (ret_val != nullptr) {
        *ret_val = ret;
    }

    if (_ball_sensed) {
        //ballSenseLED = 0;
    } else {
        //ballSenseLED = 1;
    }

    bool command_acked = command_resp == ACK;
    //LOG(DEBUG, "ACK?:%s, CMD:%02X, RET:%02X, STT:%02X",
    //    command_acked ? "true" : "false", command_resp, ret, state);

    return command_acked;
}

void KickerBoard::kickType(bool isChip) {
    if (!_is_kicking) {
        _kick_type_commanded = true;
        _is_chip = isChip;
    }
}

void KickerBoard::kick(uint8_t strength) {
    if (!_is_kicking) {
        _kick_immediate_commanded = true;
        _kick_strength = strength;
    }
}

void KickerBoard::kickOnBreakbeam(uint8_t strength) {
    if (!_is_kicking) {
        _kick_breakbeam_commanded = true;
        _kick_strength = strength;
    }
}

void KickerBoard::cancelBreakbeam() { _cancel_breakbeam_commanded = true; }

bool KickerBoard::isCharging() { return _is_charging; }

bool KickerBoard::isBallSensed() { return _ball_sensed; }

bool KickerBoard::isHealthy() { return _is_healthy; }

uint8_t KickerBoard::getVoltage() { return _current_voltage; }

bool KickerBoard::isCharged() { return getVoltage() > isChargedCutoff; }

void KickerBoard::setChargeAllowed(bool chargeAllowed) {
    if (chargeAllowed) {
        _charging_commanded = true;
    } else {
        _stop_charging_commanded = true;
    }
}
