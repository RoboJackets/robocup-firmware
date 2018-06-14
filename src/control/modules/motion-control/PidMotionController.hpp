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
        ax(0), ay(0), az(0), gx(0), gy(0), gz(0), rotation(0), target_rotation(0), angular_vel(0),
        angle_hold(false)
    { 
        setPidValues(1.0, 0, 0, 50, 0);

        rotation_pid.kp = 15;
        rotation_pid.ki = 0;
        rotation_pid.kd = 300;
        rotation_pid.setWindup(40);
        rotation_pid.derivAlpha = 0.0f; // 1 is all old, 0 is all new
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
        // update control targets
        // target_rotation += _targetVel[2] * dt;

        // get sensor data
        imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // convert sensor readings to mathematically valid values
        Eigen::Vector4d wheelVels;
        wheelVels << encoderDeltas[0], encoderDeltas[1], encoderDeltas[2], encoderDeltas[3];
        wheelVels *= 2.0 * M_PI / ENC_TICKS_PER_TURN / dt;
        
        auto bot_vel = RobotModel::get().WheelToBot * wheelVels;

        // we have two redundent sensor measurements for rotation
        // 32.8 comes from data sheet, units are LSB / (deg/s)
        float ang_vel_gyro = (gz / 32.8f) * M_PI / 180.0f;
        float ang_vel_enc = bot_vel[2];

        // printf("%f\r\n", ang_vel_enc);
        // std::printf("%f %f\r\n", ang_vel_gyro, ang_vel_enc);

        // perform sensor fusion
        // the higher this is, the more gyro measurements are used instead of encoders
        float sensor_fuse_ratio = 1;
        float ang_vel_update = ang_vel_gyro * sensor_fuse_ratio
                             + ang_vel_enc * (1 - sensor_fuse_ratio);

        // perform state update based on fused value, passed through a low passed filter
        
        // so far noise on the gyro seems pretty minimal, that's why this filter is off
        float alpha = 1.0; // 0->1 (higher means less filtering)
        angular_vel = (alpha * ang_vel_update + (1 - alpha) * angular_vel);
        // current rotation estimate
        rotation += angular_vel * dt;

        // printf("%f\r\n", rotation * 180.0f / M_PI);

        // velocity we are actually basing control off of, not the latest command
        auto target_vel_act = _targetVel;

        // std::printf("%f\r\n", rot_error);
        
        const auto epsilon = 0.0001f;
        // soccer tells us to "halt" by sending 0 vel commands, we want to freeze the
        // rotational controller too so bots dont' react when getting handled
        bool soccer_stop = (std::abs(target_vel_act[0]) < epsilon)
                           && (std::abs(target_vel_act[1]) < epsilon);
        if (!soccer_stop && std::abs(target_vel_act[2]) < epsilon) {
            
            if (!angle_hold) {
                target_rotation = rotation;
                angle_hold = true;
            }

            // get the smallest difference between two angles
            float rot_error = std::atan2(std::sin(target_rotation - rotation),
                                         std::cos(target_rotation - rotation));

            target_vel_act[2] = rotation_pid.run(rot_error);
        } else {
            // let target_vel_act be exactly what soccer commanded
            angle_hold = false;
        }

        // conversion to commanded wheel velocities
        Eigen::Vector4d targetWheelVels = RobotModel::get().BotToWheel * target_vel_act.cast<double>();

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

            if (i == 0) {
                // printf("%d\r\n", static_cast<int16_t>(dc));
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
    // We want to preserve the interface of soccer commanding rotational velocities
    // for now, so that requires us to have a separate estimate of soccer's desired rotation
    float target_rotation;
    float angular_vel;
    bool angle_hold;
};
