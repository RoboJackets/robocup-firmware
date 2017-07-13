#include "KickerBoard.hpp"
#include <tuple>

using namespace std;

std::shared_ptr<KickerBoard> KickerBoard::Instance;

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
    FILE* fp = fopen(_filename.c_str(), "r");

    if (fp == nullptr) {
        LOG(WARN, "Failed to open kicker binary, check path: '%s'",
            _filename.c_str());
        exitProgramming();
        return false;
    } else {
        // Program it!
        LOG(INFO, "Opened kicker binary, attempting to program kicker.");
        bool shouldProgram = true;
        if (onlyIfDifferent &&
            (checkMemory(ATTINY_PAGESIZE, ATTINY_NUM_PAGES, fp, false) ==
             0))
            shouldProgram = false;

        if (!shouldProgram) {
            LOG(INFO, "Kicker up-to-date, no need to flash.");

            // exit programming mode by bringing nReset high
            exitProgramming();
        } else {
            bool success =
                program(fp, ATTINY_PAGESIZE, ATTINY_NUM_PAGES);

            if (!success) {
                LOG(WARN, "Failed to program kicker.");
            } else {
                LOG(INFO, "Kicker successfully programmed.");
            }
        }

        fclose(fp);
    }

    return true;
}

bool KickerBoard::send_to_kicker(uint8_t cmd, uint8_t arg, uint8_t* ret_val) {
    LOG(DEBUG, "Sending: CMD:%02X, ARG:%02X", cmd, arg);
    wait_us(300);
    chipSelect();
    wait_us(300);
    m_spi->write(cmd);
    wait_us(300);
    uint8_t command_resp = m_spi->write(arg);
    wait_us(300);
    uint8_t ret = m_spi->write(BLANK);
    wait_us(300);
    uint8_t state = m_spi->write(BLANK);
    wait_us(300);
    chipDeselect();

    if (ret_val != nullptr) {
        *ret_val = ret;
    }

    bool command_acked = ACK;
    LOG(DEBUG, "ACK?:%s, CMD:%02X, RET:%02X, STT:%02X",
        command_acked ? "true" : "false", command_resp, ret, state);

    return command_acked;
}

bool KickerBoard::kick(uint8_t strength, bool immediate) {
    return send_to_kicker(immediate ? KICK_IMMEDIATE_CMD : KICK_BREAKBEAM_CMD,
                          strength, nullptr);
}

std::pair<bool, uint8_t> KickerBoard::readVoltage() {
    uint8_t volts;
    bool res = send_to_kicker(GET_VOLTAGE_CMD, BLANK, &volts);
    return std::make_pair(res, volts);
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
    //uint8_t ret = 0;

    /*
    chipSelect();
    ret = m_spi->write(PING_CMD);
    m_spi->write(BLANK);
    m_spi->write(BLANK);
    chipDeselect();
    */

    // boolean determined by MSB of 2nd byte
    return false;
}
