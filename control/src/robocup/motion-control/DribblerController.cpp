#include "motion-control/DribblerController.hpp"
#include <stdlib.h>

/**
 * Edited sign function to always return -1 or 1
 * 
 * Returns 1 if val >= 0, -1 if val < 0
 */
template <typename T> int sgn(T val) {
    return (T(0) <= val) - (val < T(0));
}

DribblerController::DribblerController(uint32_t dt) : pv(0), dt(10) {};

void DribblerController::calculate(uint8_t setpoint, uint8_t& command) {
    // Must be int16 since sp can go from 0 to 128 (not 127)
    int16_t diff = setpoint - pv;

    // Check if change is larger than the max change per second
    //
    // Note: multiplication in specific order to not truncate to 0
    // extra addition of 1 prevents no movement of dribbler if it does
    if (static_cast<uint16_t>(abs(diff)) > dt*MAX_DELTAV_PER_S/1000 + 1) {
        diff = sgn(diff) * (dt*MAX_DELTAV_PER_S/1000 + 1);
    }

    pv += diff;

    // Make sure still within range
    if (pv > MAX_SPEED) {
        pv = MAX_SPEED;
    } else if (pv < MIN_SPEED) {
        pv = MIN_SPEED;
    }

    // Dangerous cast, but should be fine since we limit the val
    command = static_cast<uint8_t>(pv);
}