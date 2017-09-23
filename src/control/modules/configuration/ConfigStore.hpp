#pragma once

#include "firmware-common/common2015/utils/rtp.hpp"

namespace DebugCommunication {
extern std::array<
    int16_t, static_cast<size_t>(
                 ConfigCommunication::CONFIG_COMMUNICATION_LAST_PLACEHOLDER)>
    configStore;

extern std::array<
    bool, static_cast<size_t>(
              ConfigCommunication::CONFIG_COMMUNICATION_LAST_PLACEHOLDER)>
    configStoreIsValid;

extern std::array<int16_t, static_cast<size_t>(
                               DebugResponse::DEBUG_RESPONSE_LAST_PLACEHOLDER)>
    debugStore;

extern std::array<DebugCommunication::DebugResponse,
                  rtp::RobotStatusMessage::debug_data_length> debugResponses;
}