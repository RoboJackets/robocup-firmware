#pragma once

// Whether the kick should be a chip or kick
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
#define CANCEL_KICK (0x2 << 5)

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