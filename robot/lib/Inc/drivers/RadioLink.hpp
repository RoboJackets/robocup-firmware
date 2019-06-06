#pragma once

// TODO: Figure out how to do this correctly
#include "../../lib/robocup-fshare/include/rc-fshare/rtp.hpp"
#include <memory>
#include "Internal/GenericRadio.hpp"
#include "MicroPackets.hpp"


/**
 * Sends and receives data to/from the radio
 * Converts the simplified structures to the rtp packet to send
 */
class RadioLink {
public:
    RadioLink();

    /**
     * Sends a packet of data to the radio
     * 
     * Assumes data in structs are valid
     */
    void send(const BatteryVoltage& batteryVoltage,
              const FPGAStatus& FPGAStatus,
              const KickerInfo& kickerInfo,
              const RobotID& robotID);

    /**
     * Receive the latest packet from the radio
     * Only fills data portion of structs
     * 
     * @return true if it returned a valid packet, false if there is no packet to return
     */
    bool receive(KickerCommand& kickerCommand,
                 MotionCommand& motionCommand);

private:
    std::unique_ptr<GenericRadio> radio;
};