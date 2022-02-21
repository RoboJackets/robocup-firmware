#pragma once

#include <memory>
#include <vector>
#include "fpga_bin.h"

#include "FreeRTOS.h"
#include "task.h"

#include <algorithm>
#include <stdint.h>
#include "robocup.hpp"

class FPGA {
public:
    FPGA(std::unique_ptr<SPI> spi_bus, PinName nCs, PinName initB,
         PinName progB, PinName done);

    /**
     * Configure FPGA with the "fpga_bin.h" binary
     * Must be called to initialize the fpga
     *
     * @return true if configured correctly, false if not
     *
     * @note this may return bad values
     */
    bool configure();

    /**
     * @return True if initialized and configured
     */
    bool isReady();

    /**
     * Sets the duty cycles and read the encoders for all motors
     * Also resets the watchdog on the fpga
     *
     * @param duty_cycles 5 element array specifying the duty cycle for
     *                    the specific motor
     *                    Element 1-4 are drive motors 1-4
     *                    Element 5 is the dribbler motor
     * @param size_dut Number of elements in the array
     *
     * @param enc_deltas 5 element array that will be changed to hold the
     *                   encoder counts for the 4 drive motors and the delta ticks
     *                   these correspond to
     *                   Element 1-4 are the drive motors 1-4
     *                   Element 5 is the delta tick
     * @param size_enc Number of elements in this array
     *
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     *
     *        Returns 0x7F if the MAX_DUTY_CYCLE magnitude is exceeded
     *
     * @note Both input arrays must be 5 otherwise a array out of bounds condition
     *       occurs
     */
    uint8_t set_duty_get_enc(int16_t* duty_cycles, size_t size_dut,
                             int16_t* enc_deltas, size_t size_enc);

    /**
     * Sets the duty cycles for all motors
     * Also reset the watchdog on the fpga
     *
     * @param duty_cycles 5 element array specifying the duty cycle for
     *                    the specific motor
     *                    Element 1-4 are drive motors 1-4
     *                    Element 5 is the dribbler motor
     * @param size Number of elements in the array
     *
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     *
     *        Returns 0x7F if the MAX_DUTY_CYCLE magnitude is exceeded
     */
    uint8_t set_duty_cycles(int16_t* duty_cycles, size_t size);

    /**
     * Reads the duty cycles for all motors
     *
     * @param duty_cycles 5 element array that will be changed to hold the
     *                    currently commanded duty cycles
     *                    Element 1-4 are drive motors 1-4
     *                    Element 5 is the dribbler motor
     * @param size Number of elements in the array
     *
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     */
    uint8_t read_duty_cycles(int16_t* duty_cycles, size_t size);

    /**
     * Reads the encoders for all motors
     *
     * @param enc_counts 5 element array that will be changed to hold the
     *                   encoder counts for the 4 drive motors and the delta ticks
     *                   these correspond to
     *                   Element 1-4 are the drive motors 1-4
     *                   Element 5 is the delta tick
     * @param size Number of elements in this array
     *
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     */
    uint8_t read_encs(int16_t* enc_counts, size_t size);

    /**
     * Reads the hall count for all motors (similar to encoders)
     *
     * @param halls 5 element array that will be changed to hold the
     *              hall counts for the 4 drive motors and the one dribbler motor
     *              Element 1-4 are the drive motors 1-4
     *              Element 5 is the dribbler motor
     * @param size Number of elements in this array
     *
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     */
    uint8_t read_halls(uint8_t* halls, size_t size);

    /**
     * Enables or disables the motors on the fpga
     * The on->off or off->on toggle of motors resets the watchdog
     *
     * @param state True to enable motors
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     */
    uint8_t motors_en(bool state);

    /**
     * Resets the watchdog on the fpga
     * If the watchdog is not reset, the motors stop after some time
     *
     * @return FPGA_READY       [7]   1 if fpga ready to run (no errors)
     *         WATCHDOG_TRIGGER [6]   0 if watchdog has not triggered yet
     *         MOTORS_ENABLED   [5]   1 if motors are enabled
     *         MOTOR_HAS_ERROR  [4:0] 1 to indicate error on specific motor
     *                                [4] - Dribbler
     *                                [3] - Drive motor 4
     *                                [2] - Drive motor 3
     *                                [1] - Drive motor 2
     *                                [0] - Drive motor 1
     *
     * @note Any other command that sets motor duty cycles resets the watchdog as well
     */
    uint8_t watchdog_reset();

    /**
     * Gets the git hash of the current fpga firmware
     *
     * @param v Vector that the has will be stored into. It will be 21 chars long
     *
     * @return Whether the current fpga firmware is dirty or not
     *         AKA a file has been modified but not commited
     */
    bool git_hash(std::vector<uint8_t>&);

    /**
     * Get information from each of the DRV8303's
     *
     * @param v vector that will be filled with 10 16bit status structures
     *          each halfword is structured as follows (MSB -> LSB):
     *          | nibble 3: | 0        | 0        | 0        | 0        |
     *          | nibble 2: | GVDD_OV  | FAULT    | GVDD_UV  | PVDD_UV  |
     *          | nibble 1: | OTSD     | OTW      | FETHA_OC | FETLA_OC |
     *          | nibble 0: | FETHB_OC | FETLB_OC | FETHC_OC | FETLC_OC |
     */
    void gate_drivers(std::vector<uint16_t>&);

    /**
     * Sends the config over to the FPGA
     * It is assumed that the fpga has already initialized and the spi bus is cleared out
     *
     * @return True if the config send was successful
     */
    bool send_config();

    /**
     * Bring chipselect low so we can start a transmission
     */
    void chip_select();

    /**
     * Bring chipselect high so we can end a transmission
     */
    void chip_deselect();

    static constexpr int FPGA_SPI_FREQ = 100'000;
    static const int16_t MAX_DUTY_CYCLE = 511;

private:
    bool _isInit = false;

    std::unique_ptr<SPI> _spi_bus;
    DigitalOut _nCs;
    DigitalIn _initB;
    DigitalIn _done;
    DigitalOut _progB;
};
