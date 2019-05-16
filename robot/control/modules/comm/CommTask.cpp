#include "CommTask.hpp"

#define MAX_BUFFER_SIZE 10

// Temp thing just to make sure it compiles
// Should be replaced with the real radio driver
class NewRadioDriver : public GenericRadio {
    int send(const uint8_t* data, const int numBytes) { return 0; }
    int receive(uint8_t* data, const int maxNumBytes) { return 0; }
    bool isAvailable() { return true; }
};

CommTask::CommTask() {
    radio = std::make_unique<NewRadioDriver>();

    radioCommunicator = std::thread(&CommTask::sendRecievePackets, this);
}

void CommTask::send(const KickerStatus& kickerStatus,
                    const MotorStatus& motorStatus,
                    const RobotStatus& robotStatus) {
    std::lock_guard<std::mutex> lock(sendLock);
    {
        // Make sure the buffer isn't already full
        if (sendBuffer.size() >= MAX_BUFFER_SIZE - 1) {
            // Do some warning here
            return;
        }

        std::array<uint8_t, rtp::ReverseSize> buffer;
        rtp::Header* header = reinterpret_cast<rtp::Header*>(&buffer[0]);
        rtp::RobotStatusMessage* status = reinterpret_cast<rtp::RobotStatusMessage*>(&buffer[rtp::HeaderSize]);

        header->address = rtp::BASE_STATION_ADDRESS;
        header->port    = rtp::PortType::CONTROL;
        header->type    = rtp::MessageType::CONTROL;

        uint8_t motorErrors = 0;
        for (int i = 0; i < 4; i++) 
            motorErrors |= static_cast<int>(motorStatus.motorErrors[i]) << i;

        status->uid             = robotStatus.uid;
        status->battVoltage     = robotStatus.battVoltage;
        status->motorErrors     = motorErrors;
        status->ballSenseStatus = robotStatus.ballSenseStatus;
        status->kickStatus      = kickerStatus.kickStatus;
        status->kickHealthy     = kickerStatus.kickHealthy;
        status->fpgaStatus      = robotStatus.fpgaStatus;

        for (int i = 0; i < 4; i++)
            status->encDeltas[i] = motorStatus.encDeltas[i];

        sendBuffer.push(buffer);
    }
}

bool CommTask::receive(KickerCommand& kickerCommand,
                       MotorCommand& motorCommand) {
    std::array<uint8_t, rtp::ForwardSize> p;

    std::lock_guard<std::mutex> lock(receiveLock);
    {
        // No packets to grab, zero everything and return false
        if (receiveBuffer.size() == 0) {
            
            kickerCommand.shootMode = KickerCommand::ShootMode::KICK;
            kickerCommand.triggerMode = KickerCommand::TriggerMode::OFF;
            kickerCommand.kickStrength = 0;

            motorCommand.bodyX = 0.0;
            motorCommand.bodyY = 0.0;
            motorCommand.bodyW = 0.0;
            motorCommand.dribbler = 0;

            return false;
        } else { // There's something to grab
            p = receiveBuffer.front();
            receiveBuffer.pop();
        }
    }

    // In the udp radio, only 1 robot's data is sent at a time
    // Dangerous: Should probably check size to make sure we got a valid packet
    rtp::RobotTxMessage* tx = reinterpret_cast<rtp::RobotTxMessage*>(&p[rtp::HeaderSize]);
    rtp::ControlMessage* control = reinterpret_cast<rtp::ControlMessage*>(&tx->message);

    // All possible values must be defined in the enum otherwise this is undefined behavior
    kickerCommand.shootMode = static_cast<KickerCommand::ShootMode>(control->shootMode);
    kickerCommand.triggerMode = static_cast<KickerCommand::TriggerMode>(control->triggerMode);
    kickerCommand.kickStrength = control->kickStrength;

    motorCommand.bodyX = static_cast<float>(control->bodyX / rtp::ControlMessage::VELOCITY_SCALE_FACTOR);
    motorCommand.bodyY = static_cast<float>(control->bodyY / rtp::ControlMessage::VELOCITY_SCALE_FACTOR);
    motorCommand.bodyW = static_cast<float>(control->bodyW / rtp::ControlMessage::VELOCITY_SCALE_FACTOR);
    motorCommand.dribbler = control->dribbler;

    return true;
}

void CommTask::sendRecievePackets() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(sendLock);
            if (sendBuffer.size() > 0) {

                std::array<uint8_t, rtp::ReverseSize> packet = sendBuffer.front();

                radio->send(packet.data(), rtp::ReverseSize);

                sendBuffer.pop();
            }
        }

        {
            std::lock_guard<std::mutex> lock(receiveLock);
            if (receiveBuffer.size() < MAX_BUFFER_SIZE - 1 && radio->isAvailable()) {

                std::array<uint8_t, rtp::ForwardSize> packet;

                radio->receive(packet.data(), rtp::ForwardSize);

                receiveBuffer.push(packet);
            }
        }

        // do a wait / yield
    }
}