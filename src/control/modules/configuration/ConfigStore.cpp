#include "ConfigStore.hpp"

namespace DebugCommunication {
std::array<int16_t,
           static_cast<size_t>(
               ConfigCommunication::CONFIG_COMMUNICATION_LAST_PLACEHOLDER)>
    configStore{};

std::array<bool,
           static_cast<size_t>(
               ConfigCommunication::CONFIG_COMMUNICATION_LAST_PLACEHOLDER)>
    configStoreIsValid{};

std::array<int16_t,
           static_cast<size_t>(DebugResponse::DEBUG_RESPONSE_LAST_PLACEHOLDER)>
    debugStore{};

std::array<DebugCommunication::DebugResponse,
           rtp::RobotStatusMessage::debug_data_length> debugResponses{};
}