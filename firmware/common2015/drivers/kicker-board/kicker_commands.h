#pragma once

/*
 * In order to command the KickerBoard, a command byte must be sent,
 * followed by an argument byte. The command byte will not be executed without
 * the argument, even if that argument is not really needed like in the
 * get_voltage command.
 *
 * If getting a varible, it will be returned on the spi write after the
 * command write and the argument write.
 */

/*
 * KickerBoard SPI protocol.
 * Each conversation begins with chip selecting the kickerboard.
 *
 * SPI Conversation Initiated.
 * Byte  |  Control Board   |   Kickerboard
 * ----------------------------------------------
 * 0     |    command       |   charging state
 * 1     |    argument      |   command (ack)
 * 2     |    NOP           |   command response
 * ----------------------------------------------
 */

/* Commands */
#define KICK_CMD 0x01
#define CHIP_CMD 0x02
#define SET_CHARGE_CMD 0x03
#define GET_VOLTAGE_CMD 0x04
#define PING_CMD 0x05
// command to check state of buttons
#define GET_BUTTON_STATE_CMD 0x06

/* Arguments */
#define BLANK 0x00       // Used for clarity when passing useless arguments
// Kick/Chip arguments
#define MAX_TIME_ARG 0xFF  // Used if we want to wait max time
#define DB_KICK_TIME 0x08  // Used for button press kick
#define DB_CHIP_TIME 0x08  // Used for button press chip
// Charge command arguments
#define ON_ARG 0x38        // Used for setting charge high
#define OFF_ARG 0x1A       // Used for setting charge low
// GET_BUTTON_STATE_CMD args
#define DB_CHIP_STATE 0x01
#define DB_KICK_STATE 0x02
#define DB_CHARGE_STATE 0x03

/* Response Codes, charging/not charging */
// Charging state
#define ISCHARGING 0x81
#define NOTCHARGING 0x80
