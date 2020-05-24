//motorTestLcd.h
#pragma once
#include <avr/io.h>
#include <motor-test-lcd.h>
#define F_CPU 8000000UL 
#include <util/delay.h>



#define DATA_PORT PORTD
#define DATA_DDR DDRD

#define CNTRL_PORT PORTB
#define CNTRL_DDR DDRB
#define RS_PIN 1
//#define RW_PIN NA We don't have a RW Pin
#define ENABLE_PIN 0


void LCD_goto(unsigned char y, unsigned char x);
void LCD_print(char *string);
void LCD_blink(void);
void LCD_clear(void);

void LCD_send_command(unsigned char cmnd);
void LCD_send_data(unsigned char data);
void LCD_init(void);

