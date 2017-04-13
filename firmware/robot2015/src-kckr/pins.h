#pragma once

#include <avr/io.h>

/* Inputs */
#define N_KICK_CS_PIN (PORTA7)  // pin 15
#define KICK_MOSI_PIN (PORTA6)  // pin 7, not actually used in code
#define V_MONITOR_PIN (PORTB1)  // pin 11
// active low buttons
#define DB_KICK_PIN (PORTA2)  // should send a normal kick commmand
// #define DB_CHIP_PIN (PORT)  // should send a normal chip command
#define DB_CHG_PIN (PORTA1)   // pressed = enable_charging
                           // unpressed = disable_charging
#define DB_SWITCH (PORTA3)

/* Outputs */
#define CHARGE_PIN (PORTB0)  // pin 10
// #define CHIP_PIN (PORT)    // pin 12
#define KICK_PIN (PORTA0)    // pin 13

/* Tri-State */
#define KICK_MISO_PIN (PORTA5)  // pin 8

/* Interrupts for PCMASK0 or PCMASK1 */
// #define INT_N_KICK_CS (PCINT7)
// #define INT_DB_KICK (PCINT8)
// #define INT_DB_CHIP (PCINT9)
// #define INT_DB_CHG (PCINT10)
