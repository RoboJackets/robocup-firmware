#pragma once

#include "Logger.hpp"
#include "Mbed.hpp"

class HackedKickerBoard {
public:
    // time to wait between kicks, in microseconds
    const uint32_t MIN_CHARGE_TIME = 2.5 * 1e6;

    HackedKickerBoard(DigitalOut kickLine)
        : _kickLine(kickLine),
          _kickTimer(this, &HackedKickerBoard::_stopKicking, osTimerOnce) {
        // ensure kicker is off
        _kickLine = 0;
        _lastKickTime = 0;
    }

    bool canKick() {
        return (us_ticker_read() - _lastKickTime > MIN_CHARGE_TIME);
    }

    void kick(uint8_t power) {
        uint32_t t = us_ticker_read();

        // LOG(DEBUG, "%08X - %08X = %08X : %08X", t, _lastKickTime, t -
        // _lastKickTime, MIN_CHARGE_TIME);

        // don't do anything - it hasn't charged enough since the last kick
        if (t - _lastKickTime < MIN_CHARGE_TIME) {
            return;
        }

        // LOG(DEBUG, "KICK %08X", _kickLine.read());

        _lastKickTime = t;

        // power = 255 corresponds to 8ms kick time.  Everything lower is
        // linearly scaled
        uint8_t time = (float)power / 255.0f * 8.0f;
        LOG(DEBUG, "KICK : %08X, %08X", power, time);
        if (time == 0) {
            return;
        }
        _kickTimer.start(time);

        _kickLine = 1;
    }

protected:
    void _stopKicking() { _kickLine = 0; }

private:
    DigitalOut _kickLine;
    RtosTimerHelper _kickTimer;
    uint32_t _lastKickTime;
};
