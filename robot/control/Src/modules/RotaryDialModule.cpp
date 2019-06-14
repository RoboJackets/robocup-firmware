#include "modules/RotaryDialModule.hpp"

RotaryDialModule::RotaryDialModule(std::shared_ptr<MCP23017> ioExpander, RobotID *const robotID)
    : robotID(robotID)/**, dial({})*/ {
    // todo setup dial

    robotID->isValid = false;
    robotID->lastUpdate = 0;
    robotID->robotID = 0;
}

void RotaryDialModule::entry(void) {
    robotID->isValid = true;
    robotID->lastUpdate = HAL_GetTick();
    robotID->robotID = 0;//dial.read();
}