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
    if (verbose) printf("Opening kicker firmware file...");
    FILE* fp = fopen(_filename.c_str(), "r");

    if (fp == NULL) {
        if (verbose)
            printf(
                "failed\r\nFailed to open binary. Check file path: "
                "'%s'\r\n\r\n",
                _filename.c_str());
        exitProgramming();
        return false;
    } else {
        // Program it!
        if (verbose) printf("done\r\n");

        bool shouldProgram = true;
        if (onlyIfDifferent &&
            (checkMemory(ATTINY84A_PAGESIZE, ATTINY84A_NUM_PAGES, fp, false) ==
             0))
            shouldProgram = false;

        if (!shouldProgram) {
            if (verbose)
                printf("kicker firmware is up-to-date, no need to flash\r\n");

            // exit programming mode by bringing nReset high
            exitProgramming();
        } else {
            if (verbose) printf("Starting device upload\r\n");
            bool nSuccess =
                program(fp, ATTINY84A_PAGESIZE, ATTINY84A_NUM_PAGES);

            if (verbose) printf("Device upload complete\r\n");

            if (nSuccess) {
                if (verbose) printf("FAILED\r\n");
            } else {
                if (verbose) printf("SUCCESS\r\n");
            }
        }

        fclose(fp);
    }

    return true;
}

bool KickerBoard::send_to_kicker(uint8_t cmd, uint8_t arg, uint8_t& ret_val,
                                 bool verbose) {
    chipSelect();
    // Returns state (charging, not charging), but we don't care about that
    uint8_t charge_resp = _spi->write(cmd);
    // Should return the command we just sent
    uint8_t command_resp = _spi->write(arg);
    // Should return final response to full cmd, arg pair
    ret_val = _spi->write(BLANK);
    chipDeselect();

    bool command_acked = command_resp == cmd;
    if (verbose) {
        if (!command_acked) {
            printf("Kicker failed to ack command! -- ");
        }

        printf("Kicker: CHG:%02X, CMD:%02X, RET:%02X\r\n", charge_resp,
               command_resp, ret_val);
        fflush(stdout);
    }

    return command_acked;
}

bool KickerBoard::send_to_kicker(uint8_t cmd, uint8_t arg, bool verbose) {
    uint8_t val;
    return send_to_kicker(cmd, arg, val, false);
}

bool KickerBoard::kick(uint8_t time) { return send_to_kicker(KICK_CMD, time); }

bool KickerBoard::chip(uint8_t time) { return send_to_kicker(CHIP_CMD, time); }

bool KickerBoard::read_voltage(uint8_t& voltage) {
    return send_to_kicker(GET_VOLTAGE_CMD, BLANK, voltage);
}

bool KickerBoard::charge() { return send_to_kicker(SET_CHARGE_CMD, ON_ARG); }

bool KickerBoard::stop_charging() {
    return send_to_kicker(SET_CHARGE_CMD, OFF_ARG);
}

bool KickerBoard::is_pingable() { return send_to_kicker(PING_CMD, BLANK); }

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
