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
    robotLock->robotID = -1;
}

void RotaryDialModule::start() {
    dial.init();
}

void RotaryDialModule::entry(void) {
    int new_robot_id = dial.read();

    //printf("Rotary dial: %d\r\n", new_robot_id);

    auto robotIDLock = robotID.lock();
    if (last_robot_id == new_robot_id) {
        robotIDLock->isValid = true;
        robotIDLock->lastUpdate = HAL_GetTick();
        robotIDLock->robotID = new_robot_id;
    } else {
        robotIDLock->isValid = false;
    }
    printTaskInfo();
    last_robot_id = new_robot_id;
}

void RotaryDialModule::printTaskInfo(void) {
    TaskHandle_t xHandle;
    TaskStatus_t xTaskDetails;

    xHandle = xTaskGetHandle( "RotaryDial" );

    configASSERT(xHandle);
    vTaskGetInfo(
            xHandle,
            &xTaskDetails,
            pdTRUE,
            eInvalid
            );
    printf("%c", xTaskDetails.pcTaskName);
}
