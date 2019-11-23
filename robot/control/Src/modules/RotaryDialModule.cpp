#include "modules/RotaryDialModule.hpp"
#include "iodefs.h"

RotaryDialModule::RotaryDialModule(LockedStruct<MCP23017>& ioExpander, LockedStruct<RobotID>& robotID)
    : GenericModule(kPeriod, "dial", kPriority), robotID(robotID), dial({
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT0, MCP23017::DIR_INPUT),
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT1, MCP23017::DIR_INPUT),
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT2, MCP23017::DIR_INPUT),
            IOExpanderDigitalInOut(ioExpander, HEX_SWITCH_BIT3, MCP23017::DIR_INPUT)}) {

    auto robotLock = robotID.unsafe_value();
    robotLock->isValid = false;
    robotLock->lastUpdate = 0;
    robotLock->robotID = 0;
}

void RotaryDialModule::entry(void) {
    auto robotIDLock = robotID.lock();

    if (!robotIDLock->isValid)
        robotIDLock->robotID = dial.read();
    robotIDLock->isValid = true;
    robotIDLock->lastUpdate = HAL_GetTick();
}