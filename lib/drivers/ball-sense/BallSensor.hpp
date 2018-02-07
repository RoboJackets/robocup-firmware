#pragma once

#include "Mbed.hpp"
#include "Rtos.hpp"
#include "RtosTimerHelper.hpp"

/** Determines if the emitter to reciever beam is broken while accounting for
 *  ambiant light.
 */
class BallSensor {
public:
    using CallbackSigT = void(bool hasBall);

    BallSensor(DigitalOut emitter, AnalogIn detector);

    /// Returns true if beam is broken
    bool hasBall() { return m_consecCount > HistoryThreshold; }

    /// Begin updating the sensor every @updateInterval ms
    void start(uint32_t updateInterval) { m_updateTimer.start(updateInterval); }

    /// Stop updates
    void stop() { m_updateTimer.stop(); }

    /// Set the callback to be called whenever the value of hasBall() changes
    std::function<CallbackSigT> senseChangeCallback = nullptr;

private:
    /// If the light reading and dark reading are closer than this value, the
    /// beam is considered broken
    static const auto DeltaThreshold = 500;

    /// Number of consecative "broken" senses uses for confidence region
    static const auto HistoryThreshold = unsigned(2);

    /// TODO: doc
    RtosTimerHelper m_updateTimer;

    /// TODO: doc
    AnalogIn m_detectorPin;

    /// TODO: doc
    DigitalOut m_emitterPin;

    /// Holds the light sensed when emitter is dark
    uint16_t m_senseDark = 0;

    /// Consecutive "broken" senses counter
    unsigned int m_consecCount = 0;

    /// Call periodically when the update timer fires
    void update();
};
