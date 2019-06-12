#include <stddef.h>

#include "util.h"


unsigned char EEPROM_read(unsigned int addr) {
	// Wait for completion of previous write
	while(EECR & (1 << EEWE));

	// Set up address register
	EEAR = addr;

	// Start eeprom read by writing EERE
	EECR |= (1 << EERE);

	// Return data from data register
	return EEDR;
}


void EEPROM_write(unsigned int addr, unsigned char dat) {
	// Wait for completion of previous write
	while(EECR & (1 << EEWE));

	// Set up address and data registers
	EEAR = addr;
	EEDR = dat;

	// Write logical one to EEMWE
	EECR |= (1 << EEMWE);
	
	// Start eeprom write by setting EEWE
	EECR |= (1 << EEWE);
}


EEPROM_OpStatus_t EEPROM_test_and_set(unsigned int addr, 
				  unsigned char dat,
				  unsigned char *dat_prev) {
	// read the current value
	unsigned char rv = EEPROM_read(addr);

	// current value matches desired value, nothing to do...
	if (rv == dat)
		return EEPROM_TS_SKIPPED;

	// set old value if requested
	if (dat_prev != NULL) 
		*dat_prev = rv;

	// write desired value
	EEPROM_write(addr, dat);

	// read to verify write
	unsigned char rv_verif = EEPROM_read(addr);

	// readback value still didn't match, unknown error
	if (rv_verif != dat) 
		return EEPROM_TS_UPDATE_FAILED;
		
	// hit no escape or error conditions, update successful
	return EEPROM_TS_UPDATE_OK;
}


