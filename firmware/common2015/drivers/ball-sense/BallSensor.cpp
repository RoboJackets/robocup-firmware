#include "BallSensor.hpp"

BallSensor::BallSensor(DigitalOut emitter, AnalogIn detector)
    : m_updateTimer(this, &BallSensor::update, osTimerPeriodic),
      m_detectorPin(detector),
      m_emitterPin(emitter) {}

void BallSensor::update() {
    const auto emitterOn = m_emitterPin.read();
    if (emitterOn) {
        // update value
        const auto senseLight = m_detectorPin.read_u16();
        // turn light off
        m_emitterPin.write(0);
        const auto hadBall = hasBall();

        // possible break in beam
        const auto notBrokenOrUnbroken =
            std::abs(senseLight - m_senseDark) < DeltaThreshold;
        if (notBrokenOrUnbroken) {
            ++m_consecCount;
        } else {
            m_consecCount = 0;
        }

        // callback
        if (hasBall() != hadBall && senseChangeCallback) {
            senseChangeCallback(hasBall());
        }
    } else  // Emitter off
    {
        // update value
        m_senseDark = m_detectorPin.read_u16();
        // turn light on
        m_emitterPin.write(1);
    }
}
