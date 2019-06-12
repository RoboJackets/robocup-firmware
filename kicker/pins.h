#pragma once

#include <avr/io.h>

/* core */
#define N_KICK_CS_PIN (PB4)
#define KICK_MOSI_PIN (PB5)
#define KICK_MISO_PIN (PB6)

/* hv monitoring */
#define V_MONITOR_PIN (PA6)
#define HV_IND_MAX (PA5)
#define HV_IND_HIGH (PA4)
#define HV_IND_MID (PA3)
#define HV_IND_LOW (PA2)
#define HV_IND_MIN (PA1)

/* hv reg */
#define LT_CHARGE (PD0)
#define LT_DONE_N (PD1)
#define LT_FAULT_N (PD2)

/* hv out */
#define KICK_PIN (PC4)
#define CHIP_PIN (PC5)

/* ball sense */
#define BALL_SENSE_TX (PD4)
#define BALL_SENSE_RX (PA0)
#define BALL_SENSE_LED (PD7)

/* debug */
#define DB_SWITCH (PC0)
#define DB_CHG_PIN (PC1)
#define DB_KICK_PIN (PC2)
#define DB_CHIP_PIN (PC3)

#define MCU_GREEN (PD3)
#define MCU_YELLOW (PD5)
#define MCU_RED (PD6)

