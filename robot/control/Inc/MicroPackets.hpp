#pragma once

#include "cstdint"

// Micropackets are shared memory locations for values
// where only the latest matters. There is no guarantee that
// data is not lost between reads.
//
// The "producer" of the data must initialize the micropacket
// The "consumer" of the data must check if it's valid and
// the data isn't too old (if applicable)

/** @struct MotionCommand
 * Contains robot body velocities and dribbler commands
 *
 * Passed from @ref RadioModule to @ref MotionControlModule
 */
struct MotionCommand {
    bool isValid = false; /**< Stores whether given data is valid  */
    uint32_t lastUpdate;  /**< Time that MotionCommand was sent (milliseconds) */

    float bodyXVel;       /**< x-component of robot's linear velocity (m/s) */
    float bodyYVel;       /**< y-component of robot's linear velocity (m/s) */
    float bodyWVel;       /**< Angular velocity of robot about z axis (rad/s) */

    uint8_t dribbler;     /**< 7-bit encoding of dribbler rotation (0: no spin - 128: max spin)
                          @note **This is limited from 0-128** */
};

/** @struct MotorCommand
 * Contains duty cycle commands for individual motors
 *
 * Passed from @ref MotionControlModule to @ref FPGAModule
 */
struct MotorCommand {
    bool isValid = false; /**< Stores whether given data is valid  */
    uint32_t lastUpdate;  /**< Time at which MotorCommand was last updated (milliseconds) */

    float wheels[4];      /**< % max duty cycle for each wheel motor (-1 to 1) */
    uint16_t dribbler;    /**< 7-bit encoding of dribble rotation (0: no spin - 128: max spin)
                          @note **This is limited from 0-128** */
};

/** @struct MotorFeedback
 * Contains current and encoder data for each motor
 *
 * Passed from @ref FPGAModule to @ref MotionControlModule
 */
struct MotorFeedback {
    bool isValid = false; /**< Stores whether given data is valid  */
    uint32_t lastUpdate;  /**< Time at which MotorFeedback was last updated (milliseconds) */

    float encoders[4];    /**< Encoder readings from each wheel motor (rad/s)  */
    float currents[4];    /**< Current readings from each wheel motor (amps)  */
};

/** @struct IMUData
 * Contains linear acceleration and angular velocity readings from IMU
 *
 * Passed from @ref IMUModule to @ref MotionControlModule
 */
struct IMUData {
    bool isValid = false;     /**< Stores whether given data is valid  */
    bool initialized = false; /**< Stores whether IMU has been initialized */
    uint32_t lastUpdate;      /**< Time at which IMUData was last updated (milliseconds) */

    float accelerations[3];   /**< Linear acceleration on axes [X,Y,Z] (m/s^2) */
    float omegas[3];          /**< Angular velocities on axes [X,Y,Z] (rad/s) */
};

/** @struct BatteryVoltage
 * Contains battery voltage data and whether it's at a critical level
 *
 * Passed from @ref BatteryModule to
 *  - @ref MotionControlModule
 *  - @ref LEDModule
 *  - @ref RadioModule
 */
struct BatteryVoltage {
    bool isValid = false; /**< Stores whether given data is valid  */
    uint32_t lastUpdate;  /**< Time at which BatteryVoltage was last updated (milliseconds) */

    uint8_t rawVoltage;   /**< Battery voltage (volts)  */
    bool isCritical;      /**< Stores whether battery voltage level is critical (nearly dead)  */
};

/** @struct FPGAStatus
 * Contains data on whether FPGA and motors are working or not
 *
 * Passed from @ref FPGAModule to
 *  - @ref LEDModule
 *  - @ref RadioModule
 */
struct FPGAStatus {
    bool isValid = false;     /**< Stores whether given data is valid  */
    bool initialized = false; /**< Stores whether FPGA has been initialized  */
    uint32_t lastUpdate;      /**< Time at which FPGAStatus was last updated (milliseconds) */

    bool motorHasErrors[5];   /**< Stores whether each of the motors has an error  */
    bool FPGAHasError;        /**< Stores whether FPGA has an error  */
};

/** @struct RadioError
 * Contains data on whether Radio is working properly on not
 *
 * Passed from @ref RadioModule to @ref LEDModule
 */
struct RadioError {
    bool isValid = false;     /**< Stores whether given data is valid  */
    bool initialized = false; /**< Stores whether Radio has been initialized */
    uint32_t lastUpdate;      /**< Time at which RadioError was last updated (milliseconds) */

    bool hasError;            /**< Stores whether Radio has an error */
};

/** @struct RobotID
 * Contains data on what ID was selected on the rotary dial
 *
 * Passed from @ref RotaryDialModule to @ref RadioModule
 */
struct RobotID {
    bool isValid = false; /**< Stores whether given data is valid  */
    uint32_t lastUpdate;  /**< Time at which RobotID was last updated (milliseconds) */

    uint8_t robotID;      /**< RobotID selected */
};

/** @struct KickerCommand
 * Contains data on whether to kick, chip, kick on breakbeam etc
 * Contains data on whether to kick or chip, and on which trigger
 *
 * Passed from @ref RadioModule to @ref KickerModule
 */
struct KickerCommand {
    bool isValid = false;    /**< Stores whether given data is valid  */
    uint32_t lastUpdate;     /**< Time at which MotionCommand was last updated (milliseconds) */

    enum ShootMode { KICK = 0, CHIP = 1 };
    enum TriggerMode { OFF = 0, IMMEDIATE = 1, ON_BREAK_BEAM = 2, INVALID = 3 };

    ShootMode shootMode;     /**< 1-bit encoding of whether the kicker should perform a ground kicker or a chip */
    TriggerMode triggerMode; /**< 2-bit encoding of when kicker should kick (none, immediately, on breakbeam, cancel) */

    uint8_t kickStrength;    /**< 4-bit encoding of the kick's strength (0: min strength - 15: max strength) */
};

/** @struct KickerInfo
 * Contains status on kicker and breakbeam
 *
 * Passed from @ref KickerModule to @ref RadioModule
 */
struct KickerInfo {
    bool isValid = false;     /**< Stores whether given data is valid  */
    bool initialized = false; /**< Stores whether Kicker has been initialized */
    uint32_t lastUpdate;      /**< Time at which MotionCommand was last updated (milliseconds) */

    bool kickerHasError;      /**< Stores whether Kicker has an error */
    bool kickerCharged;       /**< Stores whether Kicker is charged above appropriate threshold to kick */
    bool ballSenseTriggered;  /**< Stores whether Breakbeam is tripped */
};

/** @struct DebugInfo
 * 18 integers for debug info for any of the modules to pass back to soccer
 *
 * @note
 * Use `extern DebugInfo debugInfo;` at the top of any cpp
 * and then set specific values using `debugInfo.val[i] = X`
 *
 * @note
 * **It is up to the user to make sure the indices don't conflict**
 *
 * Passed from any module to @ref RadioModule
 */
struct DebugInfo {
    int16_t val[18]; /**< Array of integers of debug info for soccer */
};
