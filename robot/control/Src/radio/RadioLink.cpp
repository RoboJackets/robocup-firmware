#include "radio/RadioLink.hpp"

#include "mtrain.hpp"
#include "SPI.hpp"

#include "iodefs.h"
#include "drivers/ISM43340.hpp"

#include "MicroPackets.hpp"

extern DebugInfo debugInfo;

RadioLink::RadioLink() {
    std::unique_ptr radioSPI = std::make_unique<SPI>(RADIO_SPI_BUS, std::nullopt, 16'000'000);
    radio = std::make_unique<ISM43340>(std::move(radioSPI),
                                       RADIO_R0_CS,
                                       RADIO_GLB_RST,
                                       RADIO_R0_INT);
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
    status->ballSenseStatus = static_cast<uint8_t>(kickerInfo.ballSenseTriggered);
    status->kickStatus      = static_cast<uint8_t>(kickerInfo.kickerCharged);
    status->kickHealthy     = static_cast<uint8_t>(kickerInfo.kickerHasError);
    status->fpgaStatus      = static_cast<uint8_t>(fpgaStatus.FPGAHasError);

    for (int i = 0; i < 18; i++)
        status->encDeltas[i] = debugInfo.val[i];

    radio->send(packet.data(), rtp::ReverseSize);
}

bool RadioLink::receive(KickerCommand& kickerCommand,
                       MotionCommand& motionCommand) {
    // Make sure there is actually data to read
    if (!radio->isAvailable()) {
        return false;
    }

    std::array<uint8_t, rtp::ForwardSize> packet;

    if (radio->receive(packet.data(), rtp::ForwardSize) != rtp::ForwardSize) {
        // didn't get enough bytes
        return false;
    }

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
    motionCommand.bodyXVel = static_cast<float>(control->bodyX) / rtp::ControlMessage::VELOCITY_SCALE_FACTOR;
    motionCommand.bodyYVel = static_cast<float>(control->bodyY) / rtp::ControlMessage::VELOCITY_SCALE_FACTOR;
    motionCommand.bodyWVel = static_cast<float>(control->bodyW) / rtp::ControlMessage::VELOCITY_SCALE_FACTOR;
    motionCommand.dribbler = control->dribbler;

    return true;
}