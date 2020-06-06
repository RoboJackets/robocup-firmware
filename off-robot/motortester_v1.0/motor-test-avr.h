//motor-test-avr.h
#include <avr/io.h>
#include <avr/interrupt.h>
#define HIGH 5.0
#define LOW 0
#define FLT 2.5

void nextCommand(int command);
void initLCD();
void initMotorTest();
bool checkInstruction();
