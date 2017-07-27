#pragma once

/**
 * Stall detection overview
 *
 * Stall detection uses a rough estimate of the temperature of the motor
 * windings.
 * For each motor there is a stall counter.  The counter increases with motor
 * command
 * (representing applied voltage), decreases with speed (representing back-EMF,
 * airflow, and
 * energy delivered to the mechanical load), and decreases at a constant rate
 * (representing loss of heat to the environment).
 *
 * When the stall counter for a motor reaches a threshold, that motor is
 * considered to have
 * stalled until the stall flag is manually reset (or the board is reset).
 *
 * There is a possible wiring failure mode that is very important to detect:
 * If a single hall effect sensor phase is disconnected, the apparent hall state
 * may be valid but
 * incorrect, resulting in the motor being "electrically stalled".  This will
 * lead to the
 * motor windings burning out if speed control tries to drive the motor at high
 * power.
 */

template<int Stall_kSpeed, int Stall_kCommand, int Stall_Threshold, int Stall_Decay, int Stall_Deadband>
class StallDetection {
public:
    // Returns if the motor is stalled
    bool stall_update(int16_t motor_out, float wheelVel) {
        auto speed = abs(wheelVel);
        auto command = abs(motor_out);
        if (command < Stall_Deadband) {
            command = 0;
        }

        stall_counter +=
                command * Stall_kCommand - speed * Stall_kSpeed - Stall_Decay;
        if (stall_counter < 0) {
            stall_counter = 0;
        }

        if (stall_counter >= Stall_Threshold) {
            // Prevent overflow and let the fail command reset the flag
            stall_counter = Stall_Threshold;

            // Mark this motor as stalled
			// TODO enable this after it is tested.
            // stalled = true;
            stalled = false;
        }

        if (stalled && stall_counter <= Stall_Threshold/10) {
            stalled = false;
        }

        return stalled;
    }

    void resetStallDetection() {
        stalled = false;
    }

    // Public for debug purposes only
    int stall_counter = 0;
    bool stalled = false;

private:


};


typedef StallDetection<3, 1, 40000, 50, 30> WheelStallDetection;