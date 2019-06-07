#include "drivers/RadioLink.hpp"

#include "mtrain.hpp"
#include "drivers/ISM43340.hpp"

// Temp thing just to make sure it compiles
// Should be replaced with the real radio driver
class NewRadioDriver : public GenericRadio {
    int send(const uint8_t* data, const int numBytes) { return 0; }
    int receive(uint8_t* data, const int maxNumBytes) { return 0; }
    bool isAvailable() { return true; }
};

RadioLink::RadioLink() {
    // Create spi
    // Create ISM
    radioSPI = std::make_shared<SPI>(SpiBus5, std::nullopt, 1'000'000);
    radio = std::make_unique<ISM43340>(radioSPI, p17, p19, p30);
}

void RadioLink::send(const BatteryVoltage& batteryVoltage,
                     const FPGAStatus& fpgaStatus,
                     const KickerInfo& kickerInfo,
                     const RobotID& robotID) {

    std::array<uint8_t, rtp::ReverseSize> packet;
    rtp::Header* header = reinterpret_cast<rtp::Header*>(&packet[0]);
    rtp::RobotStatusMessage* status = reinterpret_cast<rtp::RobotStatusMessage*>(&packet[rtp::HeaderSize]);

    header->address = rtp::BASE_STATION_ADDRESS;
    header->port    = rtp::PortType::CONTROL;
    header->type    = rtp::MessageType::CONTROL;

    uint8_t motorErrors = 0;
    for (int i = 0; i < 4; i++) 
        motorErrors |= static_cast<int>(fpgaStatus.motorHasErrors[i]) << i;

    status->uid             = robotID.robotID;
    status->battVoltage     = batteryVoltage.rawVoltage;
    status->motorErrors     = motorErrors;
    status->ballSenseStatus = kickerInfo.ballSenseTriggered;
    status->kickStatus      = kickerInfo.kickerCharged;
    status->kickHealthy     = !kickerInfo.kickerHasError;
    status->fpgaStatus      = !fpgaStatus.FPGAHasError;

    radio->send(packet.data(), rtp::ReverseSize);
}

bool RadioLink::receive(KickerCommand& kickerCommand,
                       MotionCommand& motionCommand) {

    // Make sure there is actually data to read
    if (!radio->isAvailable()) {
        return false;
    }

    std::array<uint8_t, rtp::ForwardSize> packet;

    radio->receive(packet.data(), rtp::ForwardSize);

    // In the udp radio, only 1 robot's data is sent at a time
    // Dangerous: Should probably check size to make sure we got a valid packet
    rtp::RobotTxMessage* tx = reinterpret_cast<rtp::RobotTxMessage*>(&packet[rtp::HeaderSize]);
    rtp::ControlMessage* control = reinterpret_cast<rtp::ControlMessage*>(&tx->message);

    kickerCommand.isValid = true;
    kickerCommand.lastUpdate = HAL_GetTick();
    // All possible values must be defined in the enum otherwise this is undefined behavior
    kickerCommand.shootMode = static_cast<KickerCommand::ShootMode>(control->shootMode);
    kickerCommand.triggerMode = static_cast<KickerCommand::TriggerMode>(control->triggerMode);
    kickerCommand.kickStrength = control->kickStrength;

    motionCommand.isValid = true;
    motionCommand.lastUpdate = HAL_GetTick();
    motionCommand.bodyXVel = static_cast<float>(control->bodyX / rtp::ControlMessage::VELOCITY_SCALE_FACTOR);
    motionCommand.bodyYVel = static_cast<float>(control->bodyY / rtp::ControlMessage::VELOCITY_SCALE_FACTOR);
    motionCommand.bodyWVel = static_cast<float>(control->bodyW / rtp::ControlMessage::VELOCITY_SCALE_FACTOR);
    motionCommand.dribbler = control->dribbler;

    return true;
}