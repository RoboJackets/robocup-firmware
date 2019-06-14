#include "modules/RotaryDialModule.hpp"
#include "iodefs.h"

RotaryDialModule::RotaryDialModule(std::shared_ptr<MCP23017> ioExpander, RobotID *const robotID)
    : robotID(robotID), dial({
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT0, MCP23017::DIR_INPUT),
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT1, MCP23017::DIR_INPUT),
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT2, MCP23017::DIR_INPUT),
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT3, MCP23017::DIR_INPUT)}) {

    robotID->isValid = false;
    robotID->lastUpdate = 0;
    robotID->robotID = 0;
}

void RotaryDialModule::entry(void) {
    robotID->isValid = true;
    robotID->lastUpdate = HAL_GetTick();
    robotID->robotID = dial.read();
}