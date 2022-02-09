#include "I2C.hpp"
#include "LockedStruct.hpp"

#define TSL2572_VISIBLE 0
#define TSL2572_ADDR 0x39
#define TSL2561_LUX_CHSCALE_TINT0 (0x7517) ///< 322/11 * 2^TSL2561_LUX_CHSCALE

class TSL2572
{
public:
    /* Register Defines from Data Sheet 
    https://ams.com/documents/20143/36005/TSL2572_DS000178_4-00.pdf
    Page 19 */
    typedef enum
    {

        ENABLE = 0x00, // Enable Register - power device on/off, enable functions, and interrupts
        ATIME = 0x01,  // ALS Timing Register
        WTIME = 0x03,  // Wait Time Register
        AILTL = 0x04,  // ALS Interrupt Threshold Registers
        AILTH = 0x05,
        AIHTL = 0x06,
        AIHTH = 0x07,
        PERS = 0x0C,    // Persistence Register - controls the filter interrupt capabilities of the device
        CONFIG = 0x0D,  // Configuration Register - sets the wait long time and ALS gain level
        CONTROL = 0x0F, // Control Register - provide ALS gain control to the analog block
        ID = 0x12,      // ID Register - provides the value for the part number
        STATUS = 0x13,  // Status Register - provides the internal status of the device
        C0DATA = 0x14,  // ADC Channel Data Registers
        C0DATA = 0x14,
        C0DATAH = 0x15,
        C1DATA = 0x16,
        C1DATAH = 0x17
    } Register;

    typedef enum 
    {
        TSL2572_GAIN_1X = 0x00, //No gain 
        TSL2572_GAIN_16X = 0x10,  //16x gain 
    } tsl2572Gain_t

    typedef enum {
        I2CADDR = 0x39; 
    } Address; 

    typedef enum 
    {
        TSL2572_INTEGRATIONTIME_101MS = 0xDB, //101 ms 
    } tsl2572IntegrationTime_t

    //void writeRegister(TSL2572::Register regAddress, uint16_t data); // Declares the Write Function

    void reset(); //Declares register values for default state
    uint32_t TSL2572::calculateLux(uint16_t sensor);
    //uint16_t readRegister(TSL2572::Register regAddress);    // Declares the Read Function

private:
    boolean _tsl2572initialized;
    LockedStruct<I2C> &_i2c;
    int _i2cAddress; // physical I2C Address
    tsl2572Gain_t _tsl2572Gain; 
    tsl2572IntegrationTime_t _tsl2572IntegrationTime; 
    int32_t _tsl2572SensorID; 

    void writeRegister(uint8_t reg, uint8_t data);
    uint16_t readRegister(TSL2572::Register regAddress);
};
