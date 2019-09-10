#pragma once

// Kicker packet definition
// |---------------------------------------|
// | (7) | (6) (5) | (4) | (3) (2) (1) (0) |
// |---------------------------------------| 
//
// Bits 0-3
//  Power of kick
//      0 - 15
//      0 is min power
//      15 is max power
//
// Bits 4
//  Charge Allowed
//      Whether the kicker can start charging the caps
//      1 Charge allowed
//      0 Charge not allowed
//
// Bits 5-6
//  Type of kick activation
//      0b01 Kick on breakbeam
//      0b10 Kick immediately
//      0b11 Cancel all current kick commands
//
// Bits 7
//  Type of kick
//      1 Chip
//      0 Kick

// Whether the kick should be a chip or kick
#define TYPE_FIELD (1 << 7)
#define TYPE_KICK (0 << 7)
#define TYPE_CHIP (1 << 7)

/**
 * Only one type of kick command can be sent at once
 * If you send one, then the other, it uses the latest
 * If you send both, it cancels all kicks
 */

// Kick right now
#define KICK_IMMEDIATE (1 << 6)

// Kick on breakbeam
#define KICK_ON_BREAKBEAM (1 << 5)

// Cancel any kick commands already sent
#define CANCEL_KICK (0b11 << 5)

// Allow the kicker to charge
#define CHARGE_ALLOWED (1 << 4)

// How powerful the kick should be
#define KICK_POWER_MASK (0x0F)


// Whether the breakbeam is tripped (1) or not (0)
#define BREAKBEAM_TRIPPED (1 << 7)

// Voltage of the kicker
#define VOLTAGE_MASK   (0x7F)

// How much to multiple the voltage returned
#define VOLTAGE_SCALE (2)