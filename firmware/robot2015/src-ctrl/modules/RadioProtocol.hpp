#pragma once

#include "CC1201.hpp"
#include "CommModule.hpp"
#include "Decawave.hpp"
#include "Rtos.hpp"
#include "RtosTimerHelper.hpp"

class RadioProtocol {
public:
    enum class State {
        STOPPED,
        DISCONNECTED,
        CONNECTED,
    };

    /// After this number of milliseconds without receiving a packet from the
    /// base station, we are considered "disconnected"
    static const uint32_t TIMEOUT_INTERVAL = 2000;

    RadioProtocol(std::shared_ptr<CommModule> commModule,
                  uint8_t uid = RTP::INVALID_ROBOT_UID)
        : m_commModule(commModule),
          m_uid(uid),
          m_state(State::STOPPED),
          m_replyTimer(this, &RadioProtocol::reply, osTimerOnce),
          m_timeoutTimer(this, &RadioProtocol::timeout, osTimerOnce) {
        ASSERT(commModule != nullptr);
        ASSERT(globalRadio != nullptr);
        globalRadio->setAddress(RTP::ROBOT_ADDRESS);
    }

    ~RadioProtocol() { stop(); }

    /// Set robot unique id.  Also update address.
    void setUID(uint8_t uid) { m_uid = uid; }

    /**
     * Callback that is called whenever a packet is received.  Set this in
     * order to handle parsing the packet and creating a response.  This
     * callback
     * should return a formatted reply buffer, which will be sent in the
     * appropriate reply slot.
     *
     * @param msg A pointer to the start of the message addressed to this robot
     * @return formatted reply buffer
     */
    std::function<std::vector<uint8_t>(const RTP::ControlMessage* msg,
                                       const bool addresed)> rxCallback;

    void start() {
        m_state = State::DISCONNECTED;

        m_commModule->setRxHandler(this, &RadioProtocol::rxHandler,
                                   RTP::PortType::CONTROL);
        m_commModule->setTxHandler(globalRadio.get(), &CommLink::sendPacket,
                                   RTP::PortType::CONTROL);

        LOG(INFO, "Radio protocol listening on port %d",
            RTP::PortType::CONTROL);
    }

    void stop() {
        m_commModule->close(RTP::PortType::CONTROL);

        m_replyTimer.stop();
        m_state = State::STOPPED;

        LOG(INFO, "Radio protocol stopped");
    }

    State state() const { return m_state; }

    void rxHandler(RTP::Packet pkt) {
        // TODO: check packet size before parsing
        bool addressed = false;
        const RTP::ControlMessage* msg;
        size_t slot;
        // printf("UUIDs: ");
        for (slot = 0; slot < 6; slot++) {
            const auto offset = slot * sizeof(RTP::ControlMessage);
            msg = reinterpret_cast<const RTP::ControlMessage*>(
                pkt.payload.data() + offset);

            // printf("%d:%d ", slot, msg->uid);
            if (msg->uid == m_uid) {
                addressed = true;
                break;
            }
        }
        // printf("\r\n");

        /// time, in ms, for each reply slot
        // TODO(justin): double-check this
        const uint32_t SLOT_DELAY = 2;

        m_state = State::CONNECTED;

        // reset timeout whenever we receive a packet
        m_timeoutTimer.stop();
        m_timeoutTimer.start(TIMEOUT_INTERVAL);

        // TODO: this is bad and lazy
        if (addressed) {
            m_replyTimer.start(1 + SLOT_DELAY * slot);
        } else {
            m_replyTimer.start(1 + SLOT_DELAY * (m_uid % 6));
        }

        if (rxCallback) {
            m_reply = std::move(rxCallback(msg, addressed));
        } else {
            LOG(WARN, "no callback set");
        }
    }

private:
    void reply() {
        RTP::Packet pkt;
        pkt.header.port = RTP::PortType::CONTROL;
        pkt.header.type = RTP::MessageType::CONTROL;
        pkt.header.address = RTP::BASE_STATION_ADDRESS;

        pkt.payload = std::move(m_reply);

        m_commModule->send(std::move(pkt));
    }

    void timeout() { m_state = State::DISCONNECTED; }

    std::shared_ptr<CommModule> m_commModule;

    uint32_t m_lastReceiveTime = 0;

    uint8_t m_uid;
    State m_state;

    std::vector<uint8_t> m_reply;

    RtosTimerHelper m_replyTimer;
    RtosTimerHelper m_timeoutTimer;
};
