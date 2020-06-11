#pragma once

#include "cstdint"

/** @class Battery
 * Interfaces with analog pin to get voltage-related data
 *
 * @note As of May 2020, this class does not currently work, as we have no analog_in driver to read pin
 */
class Battery {
public:
    Battery();

    void update();             /**< Updates reading from analog pin*/

    float getBattPercentage(); /**< Returns the last battery percentage read */

    uint8_t getRaw();          /**< Returns battery voltage as a raw 8-bit integer */

    bool isBattCritical();     /**< Returns whether battery is critical (at or below 0%) */

private:
    /**
     * The maximum voltage the battery can safely hold
     *
     * Calculations:
     *  - 5-cell lipo * 4.3 v per cell = 21.5 battery volts
     *  - 68k and 10k ohm voltage divider, analog in read voltage 2.756
     *
     *  @note Voltages above this threshold indicate that the battery is overvolted!
     */
    const float MAX_SAFE_BATT_VOLTAGE_READ = 2.756;

    /**
    * The minimum voltage the battery can safely hold
    *
    * Calculations:
    *  - 5-cell lipo * 4.3 v per cell = 21.5 battery volts
    *  - 68k and 10k ohm voltage divider, analog in read voltage 1.923
    */
    const float MIN_SAFE_BATT_VOLTAGE_READ = 1.923;

    /**
     * The difference between the maximum and minimum safe voltages the battery can safely hold
     */
    const float BATT_VOLTAGE_READ_RANGE =
        (MAX_SAFE_BATT_VOLTAGE_READ - MIN_SAFE_BATT_VOLTAGE_READ);

    // AnalogIn hasn't been implemented it yet so
    // we couldn't actually do this the correct way
    // - Joe Aug 2019
    //AnalogIn battVoltageAIPin;

    float lastReadPercentage; /**< Battery percentage on last read */
    uint8_t rawVoltage;       /**< Raw battery voltage (0-255) */
};