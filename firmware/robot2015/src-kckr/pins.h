#pragma once

#include <avr/io.h>

/* Inputs */
#define N_KICK_CS_PIN (PA7)  // pin 6
#define KCKR_MOSI_PIN (PA6)  // pin 7, not actually used in code
#define V_MONITOR_PIN (PB1)  // pin 11
// active low buttons
#define DB_KICK_PIN (PA2)  // should send a normal kick commmand
#define DB_CHIP_PIN (PB2)  // should send a normal chip command
#define DB_CHG_PIN (PA1)   // pressed = enable_charging
                           // unpressed = disable_charging

/* Outputs */
#define CHARGE_PIN (PB0)  // pin 10
#define CHIP_PIN (PA1)    // pin 12
#define KICK_PIN (PA0)    // pin 13

/* Tri-State */
#define KCKR_MISO_PIN (PA5)  // pin 8

/* Interrupts for PCMASK0 or PCMASK1 */
#define INT_N_KICK_CS (PCINT7)
#define INT_DB_KICK (PCINT8)
#define INT_DB_CHIP (PCINT9)
#define INT_DB_CHG (PCINT10)
