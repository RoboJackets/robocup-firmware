#pragma once

#include <array>
#include "Pid.hpp"
#include "FPGA.hpp"
#include "RobotModel.hpp"
#include "Geometry2d/Util.hpp"

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

    // For logging
    //std::tuple<float, float> points[num_samples];
    //float points[num_samples];
    //std::vector< std::tuple<float, float> > points;
    // float points[num_samples];
    float duties[4] = {0, 0, 0, 0};

    PidMotionController() {
        setPidValues(3.0, 0, 0, 50, 0);

        //if (logging) {
        //    points.reserve(num_samples);
        //}
    }

    void log(float dt, const Eigen::Vector4f& wheelVelErr) {
        if (!logging) return;
        //printf("Hit log\r\n");

        int effective_index = cur_sample / dt_per_sample;
        if (effective_index < num_samples) {
            //points[effective_index] = std::make_tuple(dt, wheelVelErr[0]); //std::make_tuple(dt, wheelVelErr);
            // points[effective_index] = wheelVelErr[1];
            cur_sample++;
        } else {
            save_log();
        }
    }

    void save_log() {
        printf("Saving motor error log to disk.\r\n");
        //FILE *fp = fopen("/local/VEL", "w");
        //fprintf(fp, "time,m1_err,m2_err,m3_err,m4_errble\n");
        //for (auto& p : points) {
        //float time_sum = 0;
        for (int i = 0; i < num_samples; ++i) {
            //float dt; Eigen::Vector4f velErrs;
            //float dt, vel;
            //std::tie(dt, vel) = points[i];
            //time_sum += dt;
            //fprintf(fp, "%f,%f,%f,%f,%f\n", dt, velErrs[0], velErrs[1], velErrs[2], velErrs[3]);
            //fprintf(fp, "%f,%f\n", time_sum, vel);
            //fprintf(fp, "%f\n", points[i]);
            // printf("%f\r\n", points[i]);
        }
        //fprintf(fp, "%d\n", points.size());
        //fflush(fp);
        //fclose(fp);

        logging = false;
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
                               float dt, Eigen::Vector4d *errors=nullptr, Eigen::Vector4d *wheelVelsOut=nullptr, 
                               Eigen::Vector4d *targetWheelVelsOut=nullptr) {


        // convert encoder ticks to rad/s
        Eigen::Vector4d wheelVels;
        wheelVels << encoderDeltas[0], encoderDeltas[1], encoderDeltas[2],
            encoderDeltas[3];
        wheelVels *= 2.0 * M_PI / ENC_TICKS_PER_TURN / dt;
        //std::printf("%f, %d, %f\r\n", wheelVels[0], encoderDeltas[0], dt);

        /*
        static char timeBuf[25];
        auto sysTime = time(nullptr);
        localtime(&sysTime);
        */

        //std::printf("%f\r\n", _targetVel[1]);

        //std::printf("%d, %f\r\n", sysTime, dt);
        //strftime(timeBuf,25,"%H:%")

        Eigen::Vector4d wheelSlip;
        wheelSlip = RobotModel2015.SlipDetect * wheelVels;

        Eigen::Vector4d targetWheelVels =
            RobotModel2015.BotToWheel * _targetVel.cast<double>();

        // If there is some slipage
        if (!nearlyEqual(wheelSlip.squaredNorm(), 0)) {
            // Project wheel matrix onto the hyperplane without slip vector direction
            // targetWheelVels dot SlipVector * SlipVector / SlipVector.squaredNorm()
            targetWheelVels -= targetWheelVels.dot(RobotModel2015.SlipVector) * RobotModel2015.SlipVector / RobotModel2015.SlipVector.squaredNorm();
        }

        
        if (targetWheelVelsOut) {
            *targetWheelVelsOut = targetWheelVels;
        }
//        targetWheelVels *= 10;
        // Forwards
        //Eigen::Vector4f targetWheelVels(.288675, .32169, -.32169, -.288675);
        //Eigen::Vector4f targetWheelVels(.32169, .288675, -.288675, -.32169);

        /*
        float front = .34641;
        float back = .257352;
        Eigen::Vector4f targetWheelVels(front, back, -back, -front);
        */
        //Eigen::Vector4f targetWheelVels(.4, -.317803, -.476705, .6);

        // Right
        //Eigen::Vector4f targetWheelVels(.5, -.397254, -.397254, .5);
        //Eigen::Vector4f targetWheelVels(.397254, -.5, -.5, .397254);
        //Eigen::Vector4f targetWheelVels(0, 0, 0, 1);
        //targetWheelVels /= RobotModel2015.WheelRadius;

        Eigen::Vector4d wheelVelErr = targetWheelVels - wheelVels;

        if (errors) {
            *errors = wheelVelErr;
        }

        if (wheelVelsOut) {
            *wheelVelsOut = wheelVels;
        }

        // std::printf("%f\r\n", wheelVelErr[0]);


        std::array<int16_t, 4> dutyCycles;
        for (int i = 0; i < 4; i++) {
            // float dc;
            float dc = targetWheelVels[i] * RobotModel2015.DutyCycleMultiplier + copysign(4, targetWheelVels[i]);
            // int16_t dc = _controllers[i].run(wheelVelErr[i], dt);
            // dc = duties[i];
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
        }

        // if we're about to log, send 0 duty cycle
        /*
        int effective_index = cur_sample / dt_per_sample;
        if (effective_index > num_samples - 20 && effective_index < num_samples + 10) {
            for (int i = 0; i < 4; ++i) {
                dutyCycles[i] = 0;
            }
        }
        */
        //log(dt, targetVel);

// enable these printouts to get a python-formatted data set than can be
// graphed to visualize pid control and debug problems
#if 0
        printf("{\r\n");
        printf("'encDelt': [%d, %d, %d, %d],\r\n", encoderDeltas[0], encoderDeltas[1], encoderDeltas[2], encoderDeltas[3]);
        printf("'dt': %f,\r\n", dt);
        printf("'targetVel': [%f, %f, %f]", _targetVel[0], _targetVel[1], _targetVel[2]);
        printf("'wheelVels': [%f, %f, %f, %f],\r\n", wheelVels[0], wheelVels[1], wheelVels[2], wheelVels[3]);
        printf("'targetWheelVels': [%f, %f, %f, %f],\r\n", targetWheelVels[0], targetWheelVels[1], targetWheelVels[2], targetWheelVels[3]);
        printf("'duty': [%d, %d, %d, %d],\r\n", dutyCycles[0], dutyCycles[1], dutyCycles[2], dutyCycles[3]);
        printf("},\r\n");
#endif

        return dutyCycles;
    }

    // 2048 ticks per turn. Theres is a 3:1 gear ratio between the motor and the wheel.
    static const uint16_t ENC_TICKS_PER_TURN = 2048 * 3;

private:
    /// controllers for each wheel
    std::array<Pid, 4> _controllers{};

    Eigen::Vector3f _targetVel{};
};
