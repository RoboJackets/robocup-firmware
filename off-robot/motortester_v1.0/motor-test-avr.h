//motor-test-avr.h
#include <avr/io.h>
#include <avr/interrupt.h>


void nextCommand(int command);
void initLCD();
void initMotorTest();
bool checkInstruction();