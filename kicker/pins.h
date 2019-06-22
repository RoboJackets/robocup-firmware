#pragma once

#include "HAL_atmega32a.h"

/* core */
#define N_KICK_CS_PIN (PIN_B4)
#define KICK_MOSI_PIN (PIN_B5)
#define KICK_MISO_PIN (PIN_B6)

/* hv monitoring */
#define V_MONITOR_PIN (PIN_A6)
#define HV_IND_MAX    (PIN_A5)
#define HV_IND_HIGH   (PIN_A4)
#define HV_IND_MID    (PIN_A3)
#define HV_IND_LOW    (PIN_A2)
#define HV_IND_MIN    (PIN_A1)

/* hv reg */
#define LT_CHARGE  (PIN_D0)
#define LT_DONE_N  (PIN_D1)
#define LT_FAULT_N (PIN_D2)

/* hv out */
#define KICK_PIN (PIN_C4)
#define CHIP_PIN (PIN_C5)

/* ball sense */
#define BALL_SENSE_TX  (PIN_D4)
#define BALL_SENSE_RX  (PIN_A0)
#define BALL_SENSE_LED (PIN_D7)

/* debug */
#define DB_SWITCH   (PIN_C0)
#define DB_CHG_PIN  (PIN_C1)
#define DB_KICK_PIN (PIN_C2)
#define DB_CHIP_PIN (PIN_C3)

#define MCU_GREEN  (PIN_D3)
#define MCU_YELLOW (PIN_D5)
#define MCU_RED    (PIN_D6)

