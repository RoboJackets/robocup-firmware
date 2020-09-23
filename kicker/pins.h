#pragma once

#include <avr/io.h>

/* core */
#define N_KICK_CS_PIN (PB4)
#define KICK_MOSI_PIN (PB5)
#define KICK_MISO_PIN (PB6)
#define KICK_CLK_PIN  (PB7)

/* hv monitoring */
#define V_MONITOR_PIN (PA6)
#define HV_IND_MAX (PA1)
#define HV_IND_HIGH (PA2)
#define HV_IND_MID (PA3)
#define HV_IND_LOW (PA4)
#define HV_IND_MIN (PA5)

/* hv reg */
#define LT_CHARGE (PD5)
#define LT_DONE_N (PD6)
#define LT_FAULT_N (PD7)

/* hv out */
#define KICK_PIN (PB0)
#define CHIP_PIN (PB1)

/* ball sense */
#define BALL_SENSE_TX (PD4)
#define BALL_SENSE_RX (PA0)
#define BALL_SENSE_LED (PB3)

/* debug */
#define DB_SWITCH (PC0)
#define DB_CHG_PIN (PD3)
#define DB_KICK_PIN (PD2)
#define DB_CHIP_PIN (PB2)

#define MCU_GREEN (PA7)
#define MCU_YELLOW (PC7)
#define MCU_RED (PC6)

