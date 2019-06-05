#pragma once

#include <memory>
#include <queue>


#include <cmsis_os.h>
// TODO: Figure out how to do this correctly
#include "../../lib/robocup-fshare/include/rc-fshare/rtp.hpp"

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
    void send(const KickerStatus& kickerStatus,
              const MotorStatus& motorStatus,
              const RobotStatus& robotStatus);

    /**
     * Receive the latest packet from the radio
     * 
     * @return true if it returned a valid packet, false if there is no packet to return
     */
    bool receive(KickerCommand& kickerCommand,
                 MotorCommand& motorCommand);

private:
    // Note: Threading 

    // Constantly send and receive packets from the radio
    // filling the inter
    static void sendReceivePackets(void * pvParameters);

    osThreadId radioCommunicator = nullptr;

    osMutexId receiveLock;
    std::mutex sendLock;

    std::queue<std::array<uint8_t, rtp::ForwardSize>> receiveBuffer;
    std::queue<std::array<uint8_t, rtp::ReverseSize>> sendBuffer;

    std::unique_ptr<GenericRadio> radio;
};