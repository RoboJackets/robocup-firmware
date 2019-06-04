#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "lib/robocup-fshare/include/rc-fshare/rtp.hpp"

#include "CommMicroPackets.hpp"
#include "GenericRadio.hpp"


/**
 * Sends and receives data to/from the radio
 * Converts the simplified structures above to the rtp packet to send
 */
class CommTask {
public:
    CommTask();

    /**
     * Sends a packet of data to the radio
     */
    void send();

    /**
     * Receive the latest packet from the radio
     *
     * @return true if it returned a valid packet, false if there is no packet to return
     */
    bool receive();

private:
    // Constantly send and recieve packets from the radio
    // filling the inter
    void sendRecievePackets();

    std::queue<std::array<uint8_t, rtp::ForwardSize>> receiveBuffer;
    std::queue<std::array<uint8_t, rtp::ReverseSize>> sendBuffer;

    const KickerStatus* kickerStatus;
    const MotorStatus* motorStatus;
    const RobotStatus* robotStatus;

    std::unique_ptr<GenericRadio> radio;
};
