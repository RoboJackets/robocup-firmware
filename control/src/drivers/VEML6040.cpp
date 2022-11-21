

#include "LockedStruct.hpp"
#include "drivers/VEML4060.hpp"


//constructor
VEML6040::VEML6040(LockedStruct<I2C>& sharedI2C) 
    : _i2c(sharedI2C)
{
    init();
}

void VEML6040::init() {
    auto i2c_lock = _i2c.lock();

    /*
    whenever initilizing we want to put 00100000
    bits 4, 5, 6 refer to integration time setting, this is set to a mid level time setting
    bit 2 is trigger, this has no trigger and its on auto mode (bit 1), bit 7 is just 0
    Command Code is just 0 and data byte low is said above and data byte high is 0 by default
    */
    write({0x00, 0b00100000, 0x00}); 
}

uint16_t read(VEML::CommandCode colorCode) {
    auto i2c_lock = _i2c.lock();

    //new receive function being written to get the red green and blue values

    std::vector<uint_8> values = i2c_lock->transmit(colorCode);//this is where new recieve function will go

    return (uint16_t)(values[0] | (values[1] << 8));
}

//reads the value of all the colors
uint16_t read() {
    auto i2c_lock = _i2c.lock();
    
    uint16_t red = read(VEML::RED);
    uint16_t green = read(VEML::GREEN);
    uint16_t blue = read(VEML::BLUE);
    //uint16_t white = read(VEML::WHITE); not sure if this is needed to calculate RGB

    /* idk if this is the correct way to determine colors here, but I am just going
    to try and normalize the results by dividing by the sum of the red green and blue values
    but I dont think this is going to fully work because the actual high for each color is going
    to be different and certain colors are going to be biased more than others? */

    int sum = red + blue + green;
    red = red / sum;
    green = green / sum;
    blue = blue / sum;

}

//Vector of three bits, 1st: command code, 2nd: data byte low, 3rd: data byte high
void VEML6040::write(std::vector<uint8_t> data) {
    auto i2c_lock = _i2c.lock();

    //new transmit function should be written with just i2cAddress and 3 part vector for data
    i2c_lock->transmit(_i2cAddress, data);
}