#pragma once

#include "mtrain.hpp"
#include "rc-fshare/rtp.hpp"
#include "drivers/GenericRadio.hpp"
#include "MicroPackets.hpp"
#include "iodefs.h"
#include "modules/ResetModule.hpp"
#include <memory>


/**
 * Sends and receives data to/from the radio
 * Converts the simplified structures to the rtp packet to send to the radio driver
 * Reverse happens on receive, raw data -> rtp packet -> simplified structures
 */
class RadioLink {
public:
    explicit RadioLink(const ResetModule& resetModule);

    /**
     * Initialize the radio.
     */
    void init();

    /**
     * Sends a packet of data to the radio
     *
     * Assumes data in structs are valid
     */
    void send(const BatteryVoltage& batteryVoltage,
              const FPGAStatus& FPGAStatus,
              const KickerInfo& kickerInfo,
              const RobotID& robotID,
              DebugInfo& debugInfo);

    /**
     * Receive the latest packet from the radio
     * Only fills data portion of structs
     *
     * @return true if it returned a valid packet, false if there is no packet to return
     */
    bool receive(KickerCommand &kickerCommand, MotionCommand &motionCommand, LEDCommand& ledCommand);

    bool isRadioConnected() { return radioConnected; }
    bool isRadioInitialized() { return radioInitialized; }
    bool hasSoccerTimedOut() { return (cyclesWithoutPackets > 10); }
private:
    std::unique_ptr<GenericRadio> radio;
    bool radioConnected = false;
    bool radioInitialized = false;
    int cyclesWithoutPackets = 0;
};
