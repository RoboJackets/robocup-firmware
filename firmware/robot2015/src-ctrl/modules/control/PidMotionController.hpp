#pragma once

#include <array>
#include "Pid.hpp"
#include "FPGA.hpp"
#include "RobotModel.hpp"


/**
 * Robot controller that runs a PID loop on each of the four wheels.
 */
class PidMotionController {
public:
    bool logging = true;
    // num_samples * dt_per_sample * 0.005 -> 12 seconds of recording?
    const static int num_samples = 1000;
    const static int dt_per_sample = 3;
    int cur_sample = 0;

    float duties[4] = {0, 0, 0, 0};

    PidMotionController() {
        setPidValues(3.0, 0, 0, 50, 0);
    }

    void setPidValues(float p, float i, float d, unsigned int windup, float derivAlpha) {
        for (Pid& ctl : _controllers) {
            ctl.kp = p;
            ctl.ki = i;
            ctl.kd = d;
            ctl.setWindup(windup);
            ctl.derivAlpha = derivAlpha;
        }
    }

    void updatePValues(float p) {
        for (Pid& ctl : _controllers) {
            ctl.kp = p;
        }
    }

    void updateIValues(float i) {
        for (Pid& ctl : _controllers) {
            ctl.ki = i;
        }
    }

    void updateDValues(float d) {
        for (Pid& ctl : _controllers) {
            ctl.kd = d;
        }
    }

    void setGyroPid(float p, float i, float d) {
        _gyroPid.kp = p;
        _gyroPid.ki = i;
        _gyroPid.kd = d;
    }

    void setTargetVel(Eigen::Vector3f target) { _targetVel = target; }

    /**
     * Return the duty cycle values for the motors to drive at the target
     * velocity.
     *
     * @param encoderDeltas Encoder deltas for the four drive motors
     * @param dt Time in ms since the last calll to run()
     *
     * @return Duty cycle values for each of the 4 motors
     */
    std::array<int16_t, 4> run(const std::array<int16_t, 4>& encoderDeltas, float measuredTurnRate,
                               float dt, Eigen::Vector4d *errors=nullptr, Eigen::Vector4d *wheelVelsOut=nullptr, 
                               Eigen::Vector4d *targetWheelVelsOut=nullptr) {

        print_counter++;

        Eigen::Vector3f target;
        target = _targetVel;

        // if this is a float.... segfault
        double targetTurnRate = target[2];

        float gyroErr = targetTurnRate - measuredTurnRate;

        // increase or decrease target to compensate for measured feedback
        // the idea is that our normal controller will receive a falsified target
        // rate that is actually controlled by a gyro turn rate controller,
        // and hopefully this will prevent fish-tailing
        //targetTurnRate += 1.0f;//_gyroPid.run(gyroErr);
        targetTurnRate += _gyroPid.run(gyroErr);

        if (print_counter % 200 == 0) {
            printf("m: %f\r\n", measuredTurnRate);
            printf("r: %f\r\n", targetTurnRate);
            //printf("t: %f\r\n", target[2] + 1.0);
            //printf("m: %f\r\n", measuredTurnRate);
            //printf("%f\r\n", gyroErr);
            //fflush(stdout);
        }

        target[2] = targetTurnRate;

        // convert encoder ticks to rad/s
        Eigen::Vector4d wheelVels;
        wheelVels << encoderDeltas[0], encoderDeltas[1], encoderDeltas[2],
            encoderDeltas[3];
        wheelVels *= 2.0 * M_PI / ENC_TICKS_PER_TURN / dt;

        Eigen::Vector4d targetWheelVels =
            RobotModel2015.BotToWheel * target.cast<double>();

        if (targetWheelVelsOut) {
            *targetWheelVelsOut = targetWheelVels;
        }

        Eigen::Vector4d wheelVelErr = targetWheelVels - wheelVels;

        if (errors) {
            *errors = wheelVelErr;
        }

        if (wheelVelsOut) {
            *wheelVelsOut = wheelVels;
        }

        std::array<int16_t, 4> dutyCycles;
        for (int i = 0; i < 4; i++) {

            float dc = targetWheelVels[i] * RobotModel2015.DutyCycleMultiplier + copysign(4, targetWheelVels[i]);
            dc += _controllers[i].run(wheelVelErr[i]);

            if (std::abs(dc) > FPGA::MAX_DUTY_CYCLE) {
                // Limit to max duty cycle
                dc = copysign(FPGA::MAX_DUTY_CYCLE, dc);
                // Conditional integration indicating open loop control
                _controllers[i].set_saturated(true);
            } else {
                _controllers[i].set_saturated(false);
            }

            duties[i] = dc;
            dutyCycles[i] = (int16_t) dc;

            printf("dc: %d\r\n", static_cast<int16_t>(dc));

        }


        return dutyCycles;
    }

    // 2048 ticks per turn. Theres is a 3:1 gear ratio between the motor and the wheel.
    static const uint16_t ENC_TICKS_PER_TURN = 2048 * 3;

private:
    /// controllers for each wheel
    std::array<Pid, 4> _controllers{};
    Pid _gyroPid;

    int print_counter = 0;

    Eigen::Vector3f _targetVel{};
};
