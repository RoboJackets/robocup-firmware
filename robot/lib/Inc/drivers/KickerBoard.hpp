#pragma once

#include "AVR910.hpp"
#include "DigitalOut.hpp"
#include "Internal/kicker_commands.h"
#include "SPI.hpp"
#include "mtrain.hpp"

#include <string>

/**
 * A class for interfacing with the kicker board, which is based on an
 * AVR chip.
 */
class KickerBoard : public AVR910 {
public:
    /**
     * Constructor for KickerBoard
     *
     * @param spi A pointer to the shared spi bus
     * @param nCs mtrain pin for not chip select for the kicker board
     * @param nReset mtrain pin for not reset line on the ISP interface.
     * @param ball_led mtrain pin for the ball sensed led
     */
    KickerBoard(std::shared_ptr<SPI> spi, std::shared_ptr<DigitalOut> nCs, PinName nReset,
                PinName ball_led);

    /**
     * Reflashes the program on the kicker board MCU with the file
     * specified.
     *
     * @param progFilename Path to kicker program binary file that will be
     *                     loaded
     * @param onlyIfDifferent If true, compares the program file to the MCU's
     *                        flash and only goes through the flashing process
     *                        if they're different.
     * @param verbose If verbose, debug log messages are printed to stdout
     *
     * @return True if flashing was successful
     */
    bool flash(const std::string& progFilename, bool onlyIfDifferent = true,
               bool verbose = false);

    /**
     * Reflashes the program on the kicker board MCU with the default
     * binary found in the device-bins/kicker_bin.h header
     *
     * @param onlyIfDifferent If true, compares the program file to the MCU's
     *                        flash and only goes through the flashing process
     *                        if they're different.
     * @param verbose If verbose, debug log messages are printed to stdout
     *
     * @return True if flashing was successful
     */
    bool flash(bool onlyIfDifferent = true, bool verbose = false);

    /**
     * Sets whether the KickerBoard should chip or kick
     * 
     * @param isChip true to chip on next kick command,
     *               false to linear kick on next kick command
     */
    void kickType(bool isChip);

    /**
     * Sends the KickerBoard a command to kick for the allotted time in
     * in milliseconds. This roughly corresponds to kick strength.
     *
     * @param Kicker strength, eventually gets mapped to duty cycle
     */
    void kick(uint8_t strength);

    /**
     * Sends the KickerBoard a command to kick for the allotted time in
     * in milliseconds once the breakbeam triggers. This roughly corresponds
     * to kick strength.
     *
     * @param Kicker strength, eventually gets mapped to duty cycle
     */
    void kickOnBreakbeam(uint8_t strength);

    /**
     * Cancels the kick on break beam command so that it doesn't kick when
     * the break beam triggers
     */
    void cancelBreakbeam();

    /**
     * @return true when the KickerBoard is charging the caps
     */
    bool isCharging();

    /**
     * @return true if the ball triggers the breakbeam
     * 
     * @note Small low pass filter on the AVR910 that only returns true
     * after a ms or two of the same state
     */
    bool isBallSensed();

    /**
     * Reads the charge voltage back from the KickerBoard.
     *
     * @return Output voltage 0 (GND) to 255 (Vd),
     *         roughly maps to actual voltage in V
     */
    uint8_t getVoltage();

    /**
     * @return if the kicker is charged
     */
    bool isCharged();

    /**
     * Sets the charge pin (to high) and allows the caps to charge up to max voltage
     */
    void setChargeAllowed(bool chargeAllowed);

    bool isHealthy();

    /**
     * Must be called once an interation (~25hz) to communicate and update
     * the kicker
     */
    void service();

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

    std::shared_ptr<DigitalOut> _nCs;
    std::shared_ptr<SPI> _spi;
    // DigitalOut ballSenseLED;

    const uint8_t isChargedCutoff = 230;

    // Note, these fields only updated after some command is send to kicker
    bool _is_charging = false;
    bool _ball_sensed = false;
    bool _is_breakbeam_armed = false;
    bool _is_kicking = false;

    bool _is_healthy = true;

    uint8_t _current_voltage = 0;

    bool _kick_type_commanded = false;
    bool _kick_immediate_commanded = false;
    bool _kick_breakbeam_commanded = false;
    bool _cancel_breakbeam_commanded = false;
    bool _is_chip = false;
    uint8_t _kick_strength = 0;

    bool _charging_commanded = false;
    bool _stop_charging_commanded = false;

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
