#pragma once

#include <avr/io.h>

// atmega32a datasheet pg. 32 & 35 (Table 9-7)
#define SYSCLK_FUSE_CKSEL_ADDR CKSEL
#define SYSCLK_FUSE_CKSEL_VAL_8MHZ 0x04 
