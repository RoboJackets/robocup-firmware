#pragma once

#include <mbed.h>
#include <logger.hpp>
#include <string>
#include "AVR910.hpp"
#include "kicker_commands.h"

/**
 * @brief A class for interfacing with the kicker board, which is based on an
 * AVR chip.
 */
class KickerBoard : public AVR910 {
public:
    /**
     * @brief Constructor for KickerBoard
     *
     * @param sharedSPI A pointer to the shared spi bus
     * @param nReset mbed pin for not reset line on the ISP interface.
     * @param progFilename Path to kicker program binary file that will be
     *     loaded by the flash() method
     */
    KickerBoard(std::shared_ptr<SharedSPI> sharedSPI, PinName nCs,
                PinName nReset, const std::string& progFilename);

    static std::shared_ptr<KickerBoard> Instance;

    /**
     * @brief Reflashes the program on the kicker board MCU with the file
     *     specified in the constructor.
     *
     * @param onlyIfDifferent If true, compares the program file to the MCU's
     *     flash and only goes through the flashing process if they're
     *     different.
     * @param verbose If verbose, debug log messages are printed to stdout
     * @return True if flashing was successful
     */
    bool flash(bool onlyIfDifferent = true, bool verbose = false);

    /**
     * @brief Sends the KickerBoard a command to kick for the allotted time in
     *     in milliseconds. This roughly corresponds to kick strength.
     *
     * @param time Millisecond kick time, can only range from 0 to 255 ms
     * @return If the kick command was acknowledged
     */
    bool kick(uint8_t time);

    /**
     * @brief Sends the KickerBoard a command to chip for the allotted time in
     *     in milliseconds. This roughly corresponds to chip strength.
     * @param time Millisecond chip time, can only range from 0 to 255 ms
     * @return If the chip command was acknowledged
     */
    bool chip(uint8_t time);

    /**
     * @brief Reads the charge voltage back from the KickerBoard.
     * @param voltage Output voltage 0 (GND) to 255 (Vd)
     * @return If the read_voltage command was acknowledged
     */
    bool read_voltage(uint8_t* voltage);

    /**
     * @brief Sets the charge pin (to high)
     * @return If the charge command was acknowledged
     */
    bool charge();

    /**
     * @brief Clears the charge pin
     * @return If the stop_charging command was acknowledged
     */
    bool stop_charging();

    /**
     * @brief Sends a ping command and checks that we get the correct resposne
     * @return If the ping command was acknowledged
     */
    bool is_pingable();

    /**
     * @brief Gets the current charging state
     * @return True if charging is enabled, otherwise false
     */
    bool is_charge_enabled();

protected:
    /**
     * @brief Uses the given function to check if it's return value equals the
     * expected value.
     *
     * @return True if the return value was the expected value, false otherwise
     */
    bool verify_param(const char* name, char expected,
                      int (AVR910::*paramMethod)(), char mask = 0xFF,
                      bool verbose = false);

private:
    bool verbose;

    std::string _filename;

    /**
     * This function enforces the design choice that each cmd must have an arg
     * and return a value
     * @brief Send a command to the kicker.
     *
     * @param cmd Command to send
     * @param arg Command argument, ignored by kicker if command doesn't require
     * it.
     * @param ret_val Value returned by kicker
     * @param verbose Whether or not to print debug messages
     * @return Whether the command was acknowledged by the kickerboard.
     */
    bool send_to_kicker(const uint8_t cmd, const uint8_t arg, uint8_t* ret_val);
};
