#include <cmath>

#include <configuration/ConfigStore.hpp>
#include <Geometry2d/Util.hpp>
#include "Assert.hpp"
#include "Console.hpp"
#include "ConfigStore.hpp"
#include "FPGA.hpp"
#include "Logger.hpp"
#include "PidMotionController.hpp"
#include "RPCVariable.h"
#include "RobotDevices.hpp"
#include "Rtos.hpp"
#include "RtosTimerHelper.hpp"
#include "TaskSignals.hpp"
#include "io-expander.hpp"
#include "motors.hpp"
#include "mpu-6050.hpp"
#include "stall/stall.hpp"
using namespace std;

// Keep this pretty high for now. Ideally, drop it down to ~3 for production
// builds. Hopefully that'll be possible without the console
constexpr auto CONTROL_LOOP_WAIT_MS = 5;

// initialize PID controller
PidMotionController pidController;

/** If this amount of time (in ms) elapses without
 * Task_Controller_UpdateTarget() being called, the target velocity is reset to
 * zero.  This is a safety feature to prevent robots from doing unwanted things
 * when they lose radio communication.
 */
constexpr uint32_t COMMAND_TIMEOUT_INTERVAL = 250;
std::unique_ptr<RtosTimerHelper> commandTimeoutTimer = nullptr;
std::array<WheelStallDetection, 4> wheelStallDetection{};
bool commandTimedOut = true;

void Task_Controller_UpdateTarget(Eigen::Vector3f targetVel) {
    pidController.setTargetVel(targetVel);

    // reset timeout
    commandTimedOut = false;
    if (commandTimeoutTimer)
        commandTimeoutTimer->start(COMMAND_TIMEOUT_INTERVAL);
}

uint8_t dribblerSpeed = 0;
void Task_Controller_UpdateDribbler(uint8_t dribbler) {
    dribblerSpeed = dribbler;
}

/**
 * initializes the motion controller thread
 */
void Task_Controller(const void* args) {
    const auto mainID =
        reinterpret_cast<const osThreadId>(const_cast<void*>(args));

    // Store the thread's ID
    const auto threadID = Thread::gettid();
    ASSERT(threadID != nullptr);

    // Store our priority so we know what to reset it to after running a command
    const auto threadPriority = osThreadGetPriority(threadID);
    (void)threadPriority;  // disable warning if unused

#if 0 /* enable whenever the imu is actually used */
    MPU6050 imu(RJ_I2C_SDA, RJ_I2C_SCL);

    imu.setBW(MPU6050_BW_256);
    imu.setGyroRange(MPU6050_GYRO_RANGE_250);
    imu.setAcceleroRange(MPU6050_ACCELERO_RANGE_2G);
    imu.setSleepMode(false);

    char testResp;
    if ((testResp = imu.testConnection())) {
        float resultRatio[6];
        imu.selfTest(resultRatio);
        LOG(INFO,
            "IMU self test results:\r\n"
            "    Accel (X,Y,Z):\t(%2.2f%%, %2.2f%%, %2.2f%%)\r\n"
            "    Gyro  (X,Y,Z):\t(%2.2f%%, %2.2f%%, %2.2f%%)",
            resultRatio[0], resultRatio[1], resultRatio[2], resultRatio[3],
            resultRatio[4], resultRatio[5]);

        LOG(OK, "Control loop ready!\r\n    Thread ID: %u, Priority: %d",
            ((P_TCB)threadID)->task_id, threadPriority);
    } else {
        LOG(SEVERE,
            "MPU6050 not found!\t(response: 0x%02X)\r\n    Falling back to "
            "sensorless control loop.",
            testResp);
    }
#endif

    // signal back to main and wait until we're signaled to continue
    osSignalSet(mainID, MAIN_TASK_CONTINUE);
    Thread::signal_wait(SUB_TASK_CONTINUE, osWaitForever);

    std::array<int16_t, 5> duty_cycles{};

    pidController.setPidValues(3.0, 10, 2, 30, 0);

    // initialize timeout timer
    commandTimeoutTimer = make_unique<RtosTimerHelper>(
        [&]() { commandTimedOut = true; }, osTimerPeriodic);

    while (true) {
#if 0 /* enable whenever the imu is actually used */
        imu.getGyro(gyroVals);
        imu.getAccelero(accelVals);
#endif

        if (DebugCommunication::configStoreIsValid
                [DebugCommunication::ConfigCommunication::PID_P]) {
            pidController.updatePValues(DebugCommunication::configValueToFloat(
                DebugCommunication::ConfigCommunication::PID_P,
                DebugCommunication::configStore
                    [DebugCommunication::ConfigCommunication::PID_P]));
        }
        if (DebugCommunication::configStoreIsValid
                [DebugCommunication::ConfigCommunication::PID_I]) {
            pidController.updateIValues(DebugCommunication::configValueToFloat(
                DebugCommunication::ConfigCommunication::PID_I,
                DebugCommunication::configStore
                    [DebugCommunication::ConfigCommunication::PID_I]));
        }
        if (DebugCommunication::configStoreIsValid
                [DebugCommunication::ConfigCommunication::PID_D]) {
            pidController.updateDValues(DebugCommunication::configValueToFloat(
                DebugCommunication::ConfigCommunication::PID_D,
                DebugCommunication::configStore
                    [DebugCommunication::ConfigCommunication::PID_D]));
        }

        // note: the 4th value is not an encoder value.  See the large comment
        // below for an explanation.
        std::array<int16_t, 5> enc_deltas{};

        // zero out command if we haven't gotten an updated target in a while
        if (commandTimedOut) duty_cycles = {0, 0, 0, 0, 0};

        auto statusByte = FPGA::Instance->set_duty_get_enc(
            duty_cycles.data(), duty_cycles.size(), enc_deltas.data(),
            enc_deltas.size());

        /*
         * The time since the last update is derived with the value of
         * WATCHDOG_TIMER_CLK_WIDTH in robocup.v
         *
         * The last encoder reading (5th one) from the FPGA is the watchdog
         * timer's tick since the last SPI transfer.
         *
         * Multiply the received tick count by:
         *     (1/18.432) * 2 * (2^WATCHDOG_TIMER_CLK_WIDTH)
         *
         * This will give you the duration since the last SPI transfer in
         * microseconds (us).
         *
         * For example, if WATCHDOG_TIMER_CLK_WIDTH = 6, here's how you would
         * convert into time assuming the fpga returned a reading of 1265 ticks:
         *     time_in_us = [ 1265 * (1/18.432) * 2 * (2^6) ] = 8784.7us
         *
         * The precision would be in increments of the multiplier. For
         * this example, that is:
         *     time_precision = 6.94us
         *
         */
        const float dt = enc_deltas.back() * (1 / 18.432e6) * 2 * 128;

        // take first 4 encoder deltas
        std::array<int16_t, 4> driveMotorEnc;
        for (auto i = 0; i < 4; i++) driveMotorEnc[i] = enc_deltas[i];

        Eigen::Vector4d errors{};
        Eigen::Vector4d wheelVelsOut{};
        Eigen::Vector4d targetWheelVelsOut{};
        // run PID controller to determine what duty cycles to use to drive the
        // motors.
        std::array<int16_t, 4> driveMotorDutyCycles = pidController.run(
            driveMotorEnc, dt, &errors, &wheelVelsOut, &targetWheelVelsOut);

        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::PIDError0] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::PIDError0, errors[0]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::PIDError1] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::PIDError1, errors[1]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::PIDError2] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::PIDError2, errors[2]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::PIDError3] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::PIDError3, errors[3]);

        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::MotorDuty0] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::MotorDuty0,
                    duty_cycles[0]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::MotorDuty1] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::MotorDuty1,
                    duty_cycles[1]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::MotorDuty2] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::MotorDuty2,
                    duty_cycles[2]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::MotorDuty3] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::MotorDuty3,
                    duty_cycles[3]);

        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::WheelVel0] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::WheelVel0,
                    wheelVelsOut[0]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::WheelVel1] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::WheelVel1,
                    wheelVelsOut[1]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::WheelVel2] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::WheelVel2,
                    wheelVelsOut[2]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::WheelVel3] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::WheelVel3,
                    wheelVelsOut[3]);

        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::StallCounter0] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::StallCounter0,
                    wheelStallDetection[0].stall_counter);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::StallCounter1] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::StallCounter1,
                    wheelStallDetection[1].stall_counter);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::StallCounter2] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::StallCounter2,
                    wheelStallDetection[2].stall_counter);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::StallCounter3] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::StallCounter3,
                    wheelStallDetection[3].stall_counter);

        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::TargetWheelVel0] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::TargetWheelVel0,
                    targetWheelVelsOut[0]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::TargetWheelVel1] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::TargetWheelVel1,
                    targetWheelVelsOut[1]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::TargetWheelVel2] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::TargetWheelVel2,
                    targetWheelVelsOut[2]);
        DebugCommunication::debugStore
            [DebugCommunication::DebugResponse::TargetWheelVel3] =
                DebugCommunication::debugResponseToValue(
                    DebugCommunication::DebugResponse::TargetWheelVel3,
                    targetWheelVelsOut[3]);

        // assign the duty cycles, zero out motors that the fpga returns an
        // error for
        static_assert(wheelStallDetection.size() == driveMotorDutyCycles.size(),
                      "wheelStallDetection Size should be the same as "
                      "driveMotorDutyCycles");
        for (int i = 0; i < driveMotorDutyCycles.size(); i++) {
            const auto& vel = driveMotorDutyCycles[i];
            bool didStall = wheelStallDetection[i].stall_update(
                duty_cycles[i], wheelVelsOut[i]);

            const bool hasError = (statusByte & (1 << i)) || didStall;
            duty_cycles[i] = (hasError ? 0 : vel);
        }

        // limit duty cycle values, while keeping sign (+ or -)
        for (auto dc : duty_cycles) {
            if (std::abs(dc) > FPGA::MAX_DUTY_CYCLE) {
                dc = copysign(FPGA::MAX_DUTY_CYCLE, dc);
            }
        }

        // dribbler duty cycle
        duty_cycles[4] = dribblerSpeed;

        Thread::wait(CONTROL_LOOP_WAIT_MS);
    }
}
