#include "KickerBoard.hpp"
#include <tuple>

using namespace std;

std::shared_ptr<KickerBoard> KickerBoard::Instance;

KickerBoard::KickerBoard(shared_ptr<SharedSPI> sharedSPI, PinName nCs,
                         PinName nReset, const string& progFilename)
    : AVR910(sharedSPI, nCs, nReset), _filename(progFilename) {
    //this->setSPIFrequency(32000);
    serviceTimer = std::make_unique<RtosTimerHelper>([&]() { this->service(); }, osTimerPeriodic);
}

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

void KickerBoard::start() {
    serviceTimer->start(25); // 25 Hz kicker update speed
}

void KickerBoard::service() {
    // function that actually executes commands given to kicker
    wait_us(100);

    chipSelect();
    if (_kick_immediate_commanded) {
        _kick_immediate_commanded = false;
        send_to_kicker(KICK_IMMEDIATE_CMD, _kick_strength, nullptr);
        _kick_strength = 0;
    }

    if (_kick_breakbeam_commanded) {
        _kick_breakbeam_commanded = false;

        // check if it is already armed
        if (!_is_breakbeam_armed) {
            send_to_kicker(KICK_BREAKBEAM_CMD, _kick_strength, nullptr);
        }

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

    wait_us(100);
    chipDeselect();
}

bool KickerBoard::send_to_kicker(uint8_t cmd, uint8_t arg, uint8_t* ret_val) {
    LOG(DEBUG, "Sending: CMD:%02X, ARG:%02X", cmd, arg);
    wait_us(100);
    m_spi->write(cmd);
    wait_us(100);
    uint8_t command_resp = m_spi->write(arg);
    wait_us(600);
    uint8_t ret = m_spi->write(BLANK);
    wait_us(600);
    uint8_t state = m_spi->write(BLANK);
    wait_us(100);

    _is_charging = state & (1 << CHARGE_FIELD);
    _ball_sensed = state & (1 << BALL_SENSE_FIELD);
    _is_breakbeam_armed = state & (1 << KICK_ON_BREAKBEAM_FIELD);
    _is_kicking = state & (1 << KICKING_FIELD);

    if (ret_val != nullptr) {
        *ret_val = ret;
    }

    bool command_acked = command_resp == ACK;
    LOG(DEBUG, "ACK?:%s, CMD:%02X, RET:%02X, STT:%02X",
        command_acked ? "true" : "false", command_resp, ret, state);

    return command_acked;
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

void KickerBoard::cancelBreakbeam() {
    _cancel_breakbeam_commanded = true;
}

bool KickerBoard::isCharging() {
    return _is_charging;
}

bool KickerBoard::isBallSensed() {
    return _ball_sensed;
}

bool KickerBoard::isHealthy() {
    return _is_healthy;
}

uint8_t KickerBoard::getVoltage() {
    return _current_voltage;
}

void KickerBoard::setChargeAllowed(bool chargeAllowed) {
    _charging_commanded = true;
}
