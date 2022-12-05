#include "I2C.hpp"
#include "LockedStruct.hpp"

#define VEML6040A3OG_VISIBLE 0
#define VEML6040A3OG_ADDR 0xE2h
#define VEML6040A3OG_LUX_CHSCALE_TINTO 0x7517

class VEML6040A3OG
{
public:
    typedef enum
    {
        ENABLE=0x00,
        ATIME=0x01,
        WTIME=0x03,
        AILTL=0x04,
        AIHTL=0x05,
        AIHTL=0x06,
        AIHTH=0x07,
        PERS=0x0C,
        CONFIG=0x0D,
        CONTROL=0x0F,
        ID=0x12,
        STATUS=0x13,
        CODATA=0x14,
        CODATAH=0x15,
        C1DATA=0x16,
        C1DATAH=0x17
    }Register;
uint8_t I2CADDR=0x39;
typedef enum
{
    VEML6040A3OG_INTEGRATIONTIME_101MS=0xDB
}VEML6040A3OGIntegrationTime_t;

VEML6040A3OG(int32_t sensorID, I2C &sharedI2C);
void read(int sensorNum, uint8_t RGBColors[3]);
bool init();

private:
I2C &_i2c;
int32_t _VEML6040A3OGSensorID;
};