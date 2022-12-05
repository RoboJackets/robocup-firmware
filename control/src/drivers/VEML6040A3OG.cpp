#include "LockedStruct.hpp"
#include "drivers/VEML6040A3OG.hpp"
VEML6040A3OG::VEML6040A3OG(int32_t sensorID, I2C &sharedI2C)
    :_VEML6040A3OGSensorID(sensorID), _i2c(sharedI2C)
{
    this->_VEML6040A3OGSensorID=sensorID;
}

/**
 * sensorNum is an int from 1-5 which is which sensor you want to read from
 * RGBColors is an array where the read data will be stored
*/
void VEML6040A3OG::read(int sensorNum, uint8_t RGBColors[3])
{
    uint8_t lookups[6] = {-1, 4, 5, 6, 7, 3}; //lookups[sensorNum] is mux channel that corresponds to sensor number sensorNum
    //Basically, the mux has eight channels, from 0 to 7. If you send a number to the mux over i2c protocol, that will determine what channels
    //the mux sends to next. So, sending the mux the binary number 00000001 will send only to the 0th channel, whereas sending it 01000100 will
    //send to both the 2nd and the 6th channel (the channels are from right to left). So, to send something only to the channel lookups[sensorNum],
    //you need a binary number with a 1 only in the lookups[sensorNum]-th position. Essentially, since each digit in binary is 2^n, doing
    //2^lookups[sensorNum] will give a binary number where the only 1 is at lookups[sensorNum]
    _i2c.transmit(226, 2^lookups[sensorNum]); //Hexadecimal address was E2, which is (?) 226 is decimals

    //requesting and recieving red
    _i2c.transmit(16, 8); //address is 10h which is 16, data is 08h which is 8
    uint16_t red = _i2c.receive(16);

    //requesting and recieving green
    _i2c.transmit(16, 9); //address is 10h which is 16, data is 09h which is 9
    uint16_t green = _i2c.receive(16);

    //requesting and recieving blue
    _i2c.transmit(16, 10); //address is 10h which is 16, data is 0Ah which is 10
    uint16_t blue = _i2c.receive(16);

    RGBColors[0] = red;
    RGBColors[1] = green;
    RGBColors[2] = blue;
}