#pragma once

#include <array>
#include <rc-fshare/pid.hpp>
#include <rc-fshare/robot_model.hpp>

#include "FPGA.hpp"
#include "MPU6050.h"
#include "RobotDevices.hpp"

/**
 * Robot controller that runs a PID loop on each of the four wheels.
 */
class PidMotionController {
public:
    PidMotionController()
    {
        setPidValues(1.0, 0, 0, 50, 0);

        rotation_pid.kp = 40;
        rotation_pid.ki = 10;
        rotation_pid.kd = 0;
        rotation_pid.setWindup(100);
        rotation_pid.derivAlpha = 0.0f;  // 1 is all old, 0 is all new
    }

    void setPidValues(float p, float i, float d, unsigned int windup,
                      float derivAlpha) {
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

    void setTargetVel(Eigen::Vector3f target) { _targetVel = target; }

    /**
     * Return the duty cycle values for the motors to drive at the target
     * velocity.
     *
     * @param measCurrentsMx the three measured currents from each of the 4 wheels
     * @param dt Time in ms since the last calll to run()
     *
     * @return Duty cycle values for each of the 4 motors
     */
    std::array<int16_t, 4> run(const std::array<int16_t, 3>& measCurrentsM1,
                                const std::array<int16_t, 3>& measCurrentsM2,
                                const std::array<int16_t, 3>& measCurrentsM3,
                                const std::array<int16_t, 3>& measCurrentsM4
                               float dt, Eigen::Vector4d* errors = nullptr,
                               Eigen::Vector4d* wheelVelsOut = nullptr,
                               Eigen::Vector4d* targetCurrent= nullptr)
    {

        std::array<float, 4> maxCurrents[0] = max({measCurrentsM1[0], measCurrentsM1[1], measCurrentsM1[2]});
        std::array<float, 4> maxCurrents[1] = max({measCurrentsM2[0], measCurrentsM2[1], measCurrentsM2[2]});
        std::array<float, 4> maxCurrents[2] = max({measCurrentsM3[0], measCurrentsM3[1], measCurrentsM3[2]});
        std::array<float, 4> maxCurrents[3] = max({measCurrentsM4[0], measCurrentsM4[1], measCurrentsM4[2]});

        std::array<float, 4> error;
        std::array<float, 4> dutyCycles;

        for (int i = 0; i < 4; i++)
        {
            error[i] = target_rotation(i) - maxCurrents[i];
            dutyCycles[i] = current_pid.run(error[i]);
        }
        return dutyCycles;

    }


private:
    Pid current_pid;
};
