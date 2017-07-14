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
 * THIS MAY BE INACCURATE
 * KickerBoard SPI protocol.
 * Each conversation begins with chip selecting the kickerboard.
 *
 * SPI Conversation Initiated.
 * Byte  |  Control Board   |   Kickerboard
 * ----------------------------------------------
 * 0     |    command       |   *
 * 1     |    argument      |   command acknowledgement
 * 2     |    *             |   command response
 * ----------------------------------------------
 */

#define CHARGE_FIELD 0x01
#define BALL_SENSE_FIELD 0x02

/* Commands */
#define KICK_IMMEDIATE_CMD 0x01
#define KICK_BREAKBEAM_CMD 0x02
#define KICK_BREAKBEAM_CANCEL_CMD 0x03
#define SET_CHARGE_CMD 0x04
#define GET_VOLTAGE_CMD 0x05
#define PING_CMD 0x06

/* Arguments */
#define BLANK 0x00  // Used for clarity when passing useless arguments
// Kick/Chip arguments
#define MAX_TIME_ARG 0xFF  // Used if we want to wait max time
#define DB_KICK_TIME 8  // Used for button press kick
#define DB_CHIP_TIME 8  // Used for button press chip
// Charge command arguments
#define ON_ARG 0x38   // Used for setting charge high
#define OFF_ARG 0x1A  // Used for setting charge low

/* Response Codes, charging/not charging */
#define ACK 0x77;
