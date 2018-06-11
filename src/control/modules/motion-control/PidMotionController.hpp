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
    PidMotionController() : imu(shared_i2c, MPU6050_DEFAULT_ADDRESS),
        ax_offset(0), ay_offset(0), az_offset(0), gx_offset(0), gy_offset(0), gz_offset(0),
        ax(0), ay(0), az(0), gx(0), gy(0), gz(0), rotation(0), angular_vel(0)
    { 
        setPidValues(3.0, 0, 0, 50, 0);

        rotation_pid.kp = 5;
        rotation_pid.ki = 0;
        rotation_pid.kd = 0;
    }

    // can't init gyro in constructor because i2c not fully up?
    void startGyro(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz) {
        imu.initialize();

        // Thread::wait(100);

        imu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
        imu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

        imu.setXAccelOffset(ax);
        imu.setYAccelOffset(ay);
        imu.setZAccelOffset(az);
        imu.setXGyroOffset(gx);
        imu.setYGyroOffset(gy);
        imu.setZGyroOffset(gz);
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
     * @param encoderDeltas Encoder deltas for the four drive motors
     * @param dt Time in ms since the last calll to run()
     *
     * @return Duty cycle values for each of the 4 motors
     */
    std::array<int16_t, 4> run(const std::array<int16_t, 4>& encoderDeltas,
                               float dt, Eigen::Vector4d* errors = nullptr,
                               Eigen::Vector4d* wheelVelsOut = nullptr,
                               Eigen::Vector4d* targetWheelVelsOut = nullptr) {
        imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // convert sensor readings to mathematically valid values
        Eigen::Vector4d wheelVels;
        wheelVels << encoderDeltas[0], encoderDeltas[1], encoderDeltas[2], encoderDeltas[3];
        wheelVels *= 2.0 * M_PI / ENC_TICKS_PER_TURN / dt;
        // auto bot_vel = RobotModel::get().EncToBot * wheelVels;
        // body_vels = body_vels / dt;
        
        auto bot_vel = RobotModel::get().WheelToBot * wheelVels;

        // two redundent sensor measurements for rotation
        // 32.8 comes from data sheet, units are LSB / (deg/s)
        float ang_vel_gyro = (gz / 32.8f) * M_PI / 180.0f;
        float ang_vel_enc = bot_vel[2];
        // float ang_vel_enc = dt;

        // printf("%f\r\n", ang_vel_enc);

        // std::printf("%f %f\r\n", ang_vel_gyro, ang_vel_enc);

        // perform sensor fusion
        // the higher this is, the more gyro measurements are used instead of encoders
        float sensor_fuse_ratio = 0;
        float ang_vel_update = ang_vel_gyro * sensor_fuse_ratio
                             + ang_vel_enc * (1 - sensor_fuse_ratio);

        // perform state update based on fused value
        float alpha = 0.8;
        angular_vel = (alpha * ang_vel_update + (1 - alpha) * angular_vel);
        rotation += angular_vel * dt;

        // printf("%f\r\n", rotation);

        // rotation controller
        float target_w = _targetVel[2];
        target_w = rotation_pid.run(rotation); // rotation pid to do an angular hold
        // correct target velocity to include rotation hold
        _targetVel[2] += target_w;

        // conversion to commanded wheel velocities
        Eigen::Vector4d targetWheelVels = RobotModel::get().BotToWheel * _targetVel.cast<double>();

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

        // Calculated by checking for slippage at max accel, and decreasing
        // appropriately
        // Binary search works really well in this case
        // Caution: This is dependent on the PID values so increasing the
        // agressiveness of that will change this
        double max_error = 3.134765625;
        double scale = 1;

        for (int i = 0; i < 4; i++) {
            if (abs(wheelVelErr[i]) > max_error) {
                scale = max(scale, abs(wheelVelErr[i]) / max_error);
            }
        }

        wheelVelErr /= scale;
        targetWheelVels = wheelVels + wheelVelErr;

        std::array<int16_t, 4> dutyCycles;
        for (int i = 0; i < 4; i++) {
            float dc =
                targetWheelVels[i] * RobotModel::get().DutyCycleMultiplier +
                copysign(4, targetWheelVels[i]);

            dc += _controllers[i].run(wheelVelErr[i]);

            if (std::abs(dc) > FPGA::MAX_DUTY_CYCLE) {
                // Limit to max duty cycle
                dc = copysign(FPGA::MAX_DUTY_CYCLE, dc);
                // Conditional integration indicating open loop control
                _controllers[i].set_saturated(true);
            } else {
                _controllers[i].set_saturated(false);
            }

            dutyCycles[i] = (int16_t)dc;
        }

        return dutyCycles;
    }

    // 2048 ticks per turn. Theres is a 3:1 gear ratio between the motor and the
    // wheel.
    static const uint16_t ENC_TICKS_PER_TURN = 2048 * 3;

private:
    /// controllers for each wheel
    std::array<Pid, 4> _controllers{};

    Pid rotation_pid;

    Eigen::Vector3f _targetVel{};

    MPU6050 imu;

    int ax_offset, ay_offset, az_offset,
        gx_offset, gy_offset, gz_offset;
    int16_t ax, ay, az,
            gx, gy, gz;

    float rotation; // state estimate for rotation
    float angular_vel;
};
