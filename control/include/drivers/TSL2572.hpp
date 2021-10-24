#include "I2C.hpp"
#include "LockedStruct.hpp"

class TSL2572 {
public:
    /* Register Defines from Data Sheet 
    https://ams.com/documents/20143/36005/TSL2572_DS000178_4-00.pdf
    Page 19 */
    typedef enum {
        
        ENABLE = 0x00, // Enable Register - power device on/off, enable functions, and interrupts
        ATIME = 0x01,  // ALS Timing Register 
        WTIME = 0x03,  // Wait Time Register
        AILTL = 0x04,  // ALS Interrupt Threshold Registers
        AILTH = 0x05,
        AIHTL = 0x06,
        AIHTH = 0x07,  
        PERS = 0x0C,   // Persistence Register - controls the filter interrupt capabilities of the device
        CONFIG = 0x0D, // Configuration Register - sets the wait long time and ALS gain level
        CONTROL = 0x0F,// Control Register - provide ALS gain control to the analog block
        ID = 0x12,     // ID Register - provides the value for the part number
        STATUS = 0x13, // Status Register - provides the internal status of the device
        C0DATA = 0x14, // ADC Channel Data Registers
        C0DATA = 0x14,
        C0DATAH = 0x15,
        C1DATA = 0x16,
        C1DATAH = 0x17
        
        } Register;

    void writeRegister(TSL2572::Register regAddress, uint16_t data); // Declares the Write Function
    uint16_t TSL2572::readRegister(TSL2572::Register regAddress);    // Declares the Read Function

private:
    LockedStruct<I2C>& _i2c;
    int _i2cAddress; // physical I2C Address

};