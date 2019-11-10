#pragma once

#include "HAL_attiny167.h"

/*
VMON        PIN_A0
n btn chg   PIN_A1
miso        PIN_A2
nbtnkck     PIN_A3
mosi        PIN_A4
sck         PIN_A5
n kicker cs PIN_A6
n btn chp   PIN_A7
chip        PIN_B0
kck         PIN_B1
charge      PIN_B2
bb rx       PIN_B3
bb tx       PIN_B4
mode select PIN_B5
n reset     pb7
*/

/* core */
#define N_KICK_CS_PIN (PIN_A6)
#define KICK_MOSI_PIN (PIN_A4)
#define KICK_MISO_PIN (PIN_A2)
#define KICK_SCK_PIN  (PIN_A5)

/* hv monitoring */
#define V_MONITOR_PIN (PIN_A0)

/* hv reg */
#define LT_CHARGE  (PIN_B2)

/* hv out */
#define KICK_PIN (PIN_B1)
#define CHIP_PIN (PIN_B0)

/* ball sense */
#define BALL_SENSE_TX  (PIN_B4)
#define BALL_SENSE_RX  (PIN_B3)

/* debug */
#define DB_SWITCH   (PIN_B5)
#define DB_CHG_PIN  (PIN_A1)
#define DB_KICK_PIN (PIN_A3)
#define DB_CHIP_PIN (PIN_A7)
