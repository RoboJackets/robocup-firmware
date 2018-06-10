#include <cmath>

#include "Assert.hpp"
#include "Battery.hpp"
#include "Console.hpp"
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
#include "stall/stall.hpp"

// some versions of gcc don't have std::round despite compiling c++11?
#define EIGEN_HAS_CXX11_MATH 0
// #include <Eigen/DisableStupidWarnings.h>
#include <Eigen/Dense>

// Keep this pretty high for now. Ideally, drop it down to ~3 for production
// builds. Hopefully that'll be possible without the console
constexpr auto CONTROL_LOOP_WAIT_MS = 50;

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

std::array<int16_t, 4> enc_counts = {0, 0, 0, 0};

void Task_Controller_UpdateTarget(Eigen::Vector3f targetVel) {
    pidController.setTargetVel(targetVel);

    // reset timeout
    commandTimedOut = false;
    if (commandTimeoutTimer)
        commandTimeoutTimer->start(COMMAND_TIMEOUT_INTERVAL);
}

void Task_Controller_UpdateOffsets(int16_t ax, int16_t ay, int16_t az,
                                   int16_t gx, int16_t gy, int16_t gz) {
    pidController.startGyro(ax, ay, az, gx, gy, gz);
}

constexpr uint8_t DRIBBLER_SPEED_UPPERBOUND = 128;
constexpr uint8_t DRIBBLER_SPEED_LOWERBOUND = 0;
constexpr float DRIBBLER_FULL_RAMP_TIME_MS = 500;
constexpr uint8_t DRIBBLER_MAX_DELTAV_PER_ITER = static_cast<uint8_t>(
    (static_cast<float>(DRIBBLER_SPEED_UPPERBOUND - DRIBBLER_SPEED_LOWERBOUND) /
     (DRIBBLER_FULL_RAMP_TIME_MS / static_cast<float>(CONTROL_LOOP_WAIT_MS))) +
    1.0f);

uint8_t dribblerSpeed = 0;
uint8_t dribblerSpeedSetPoint = 0;
void Task_Controller_UpdateDribbler(uint8_t dribbler) {
    dribblerSpeedSetPoint = dribbler;
}

/**
 *
 */
uint8_t get_damped_drib_duty_cycle() {
    // if we're already at the speed we want, return
    if (dribblerSpeed == dribblerSpeedSetPoint) {
        return dribblerSpeed;
    }

    // overflows/underflows on unsigned ints don't behave the way we want
    // a cast is cheaper than more logic based on the reg size of the M3
    int16_t sDribblerSpeed = static_cast<int16_t>(dribblerSpeed);
    int16_t sDribblerSpeedSetPoint =
        static_cast<int16_t>(dribblerSpeedSetPoint);

    if (dribblerSpeed < dribblerSpeedSetPoint) {  // ramp up
        if (sDribblerSpeed + DRIBBLER_MAX_DELTAV_PER_ITER >=
            sDribblerSpeedSetPoint) {
            dribblerSpeed = dribblerSpeedSetPoint;
        } else {
            dribblerSpeed += DRIBBLER_MAX_DELTAV_PER_ITER;
        }
    } else {  // ramp down
        if (sDribblerSpeed - DRIBBLER_MAX_DELTAV_PER_ITER <=
            sDribblerSpeedSetPoint) {
            dribblerSpeed = dribblerSpeedSetPoint;
        } else {
            dribblerSpeed -= DRIBBLER_MAX_DELTAV_PER_ITER;
        }
    }

    return dribblerSpeed;
}

std::array<int16_t, 4> Task_Controller_EncGetClear() {
    // copy
    std::array<int16_t, 4> ret_enc(enc_counts);
    // perform reset
    for (auto& e : enc_counts) {
        e = 0;
    }
    return ret_enc;
}

/**
 * initializes the motion controller thread
 */
void Task_Controller(const void* args) {
    const auto mainID = reinterpret_cast<osThreadId>(const_cast<void*>(args));

    // Store the thread's ID
    const auto threadID = Thread::gettid();
    ASSERT(threadID != nullptr);

    // Store our priority so we know what to reset it to after running a command
    const auto threadPriority = osThreadGetPriority(threadID);
    (void)threadPriority;  // disable warning if unused

    // signal back to main and wait until we're signaled to continue
    osSignalSet(mainID, MAIN_TASK_CONTINUE);
    Thread::signal_wait(SUB_TASK_CONTINUE, osWaitForever);

    std::array<int16_t, 5> duty_cycles{};

    pidController.setPidValues(3.0, 10, 2, 30, 0);

    // initialize timeout timer
    commandTimeoutTimer = make_unique<RtosTimerHelper>(
        [&]() { commandTimedOut = true; }, osTimerPeriodic);

    while (true) {
        // note: the 4th value is not an encoder value.  See the large comment
        // below for an explanation.
        std::array<int16_t, 5> enc_deltas{};

        // zero out command if we haven't gotten an updated target in a while
        if (commandTimedOut || Battery::globBatt->isBattCritical()) {
            duty_cycles = {0, 0, 0, 0, 0};
        }

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
        for (auto i = 0; i < 4; i++) {
            driveMotorEnc[i] = enc_deltas[i];
            enc_counts[i] += enc_deltas[i];
        }

        Eigen::Vector4d errors{};
        Eigen::Vector4d wheelVelsOut{};
        Eigen::Vector4d targetWheelVelsOut{};
        // run PID controller to determine what duty cycles to use to drive the
        // motors.
        std::array<int16_t, 4> driveMotorDutyCycles = pidController.run(
            driveMotorEnc, dt, &errors, &wheelVelsOut, &targetWheelVelsOut);

        // assign the duty cycles, zero out motors that the fpga returns an
        // error for
        static_assert(wheelStallDetection.size() == driveMotorDutyCycles.size(),
                      "wheelStallDetection Size should be the same as "
                      "driveMotorDutyCycles");
        for (std::size_t i = 0; i < driveMotorDutyCycles.size(); i++) {
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
        // duty_cycles[4] = dribblerSpeed;
        duty_cycles[4] = get_damped_drib_duty_cycle();

        Thread::wait(CONTROL_LOOP_WAIT_MS);
    }
}
