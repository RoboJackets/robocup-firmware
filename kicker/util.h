#pragma once

#include <avr/io.h>

typedef enum EEPROM_OpStatus {
	EEPROM_TS_UPDATE_OK = 0,
	EEPROM_TS_UPDATE_FAILED,
	EEPROM_TS_SKIPPED
} EEPROM_OpStatus_t;

unsigned char EEPROM_read(unsigned int addr);
void EEPROM_write(unsigned int addr, unsigned char dat);
EEPROM_OpStatus_t EEPROM_test_and_set(unsigned int addr,
				      unsigned char dat,
				      unsigned char *dat_prev);
