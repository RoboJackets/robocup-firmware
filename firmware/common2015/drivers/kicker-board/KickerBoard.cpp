#include "KickerBoard.hpp"
#include <tuple>

using namespace std;

KickerBoard::KickerBoard(shared_ptr<SharedSPI> sharedSPI, PinName nCs,
                         PinName nReset, const string& progFilename)
    : AVR910(sharedSPI, nCs, nReset), _filename(progFilename) {}

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

bool KickerBoard::flash(bool onlyIfDifferent, bool verbose) {
    // Check a few parameters before attempting to flash to ensure that we have
    // the right chip and it's connected correctly.
    auto checks = {
        make_tuple("Vendor ID", ATMEL_VENDOR_CODE, &AVR910::readVendorCode,
                   0xFF),
        make_tuple("Part Family", AVR_FAMILY_ID,
                   &AVR910::readPartFamilyAndFlashSize, AVR_FAMILY_MASK),
        make_tuple("Device ID", ATTINY84A_DEVICE_ID, &AVR910::readPartNumber,
                   0xFF),
    };
    for (auto& check : checks) {
        if (!verify_param(get<0>(check), get<1>(check), get<2>(check),
                          get<3>(check), verbose)) {
            return false;
        }
    }

    //  Open binary file to write to AVR.
    FILE* fp = fopen(_filename.c_str(), "r");

    if (fp == nullptr) {
        LOG(WARN, "Failed to open kicker binary, check path: '%s'",
            _filename.c_str());
        exitProgramming();
        return false;
    } else {
        // Program it!
        LOG(INIT, "Opened kicker binary, attempting to program kicker.");
        bool shouldProgram = true;
        if (onlyIfDifferent &&
            (checkMemory(ATTINY84A_PAGESIZE, ATTINY84A_NUM_PAGES, fp, false) ==
             0))
            shouldProgram = false;

        if (!shouldProgram) {
            LOG(INIT, "Kicker up-to-date, no need to flash.");

            // exit programming mode by bringing nReset high
            exitProgramming();
        } else {
            bool nSuccess =
                program(fp, ATTINY84A_PAGESIZE, ATTINY84A_NUM_PAGES);

            if (nSuccess) {
                LOG(WARN, "Failed to program kicker.");
            } else {
                LOG(INIT, "Kicker successfully programmed.");
            }
        }

        fclose(fp);
    }

    return true;
}

bool KickerBoard::send_to_kicker(uint8_t cmd, uint8_t arg, uint8_t* ret_val) {
    chipSelect();
    // Returns state (charging, not charging), but we don't care about that
    uint8_t charge_resp = _spi->write(cmd);
    // Should return the command we just sent
    uint8_t command_resp = _spi->write(arg);
    // Should return final response to full cmd, arg pair
    uint8_t ret = _spi->write(BLANK);
    chipDeselect();

    if (ret_val != nullptr) {
        *ret_val = ret;
    }

    bool command_acked = command_resp == cmd;
    LOG(INF2, "ACK?:%s, CHG:%02X, CMD:%02X, RET:%02X",
        command_acked ? "true" : "false", charge_resp, command_resp, ret);

    return command_acked;
}

bool KickerBoard::kick(uint8_t time) {
    return send_to_kicker(KICK_CMD, time, nullptr);
}

bool KickerBoard::chip(uint8_t time) {
    return send_to_kicker(CHIP_CMD, time, nullptr);
}

bool KickerBoard::read_voltage(uint8_t* voltage) {
    return send_to_kicker(GET_VOLTAGE_CMD, BLANK, voltage);
}

bool KickerBoard::charge() {
    return send_to_kicker(SET_CHARGE_CMD, ON_ARG, nullptr);
}

bool KickerBoard::stop_charging() {
    return send_to_kicker(SET_CHARGE_CMD, OFF_ARG, nullptr);
}

bool KickerBoard::is_pingable() {
    return send_to_kicker(PING_CMD, BLANK, nullptr);
}

bool KickerBoard::is_charge_enabled() {
    uint8_t ret = 0;

    chipSelect();
    ret = _spi->write(PING_CMD);
    _spi->write(BLANK);
    _spi->write(BLANK);
    chipDeselect();

    // boolean determined by MSB of 2nd byte
    return ret == ISCHARGING;
}
