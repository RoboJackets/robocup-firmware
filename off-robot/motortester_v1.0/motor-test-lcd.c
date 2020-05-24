//motor-test-lcd.c
#include <motor-test-lcd.h>

// int main (void) {
// 	CNTRL_DDR = 0xFF;
// 	CNTRL_PORT = 0x00;
// 	DATA_DDR = 0xFF;
// 	DATA_PORT = 0x00;

// 	LCD_clear();
// 	LCD_init();
// 	LCD_goto(1,2);
// 	LCD_Print("Blah");
// 	LCD_goto(2,3);
// 	LCD_print("Blah 2");
// 	while(1) {
// 		LCD_blink();
// 	}
// }

void LCD_init() {
	_delay_ms(10);
	LCD_send_command(0x38);
	LCD_send_command(0x0E);
	LCD_send_command(0x01);
	_delay_ms(10);
	LCD_send_command(0x06);
	CNTRL_DDR = 0xFF;
	CNTRL_PORT = 0x00;
	DATA_DDR = 0xFF;
 	DATA_PORT = 0x00;

}
void LCD_send_command(unsigned char cmnd) {
	DATA_PORT = cmnd;
	CNTRL_PORT &= ~(1<<RW_PIN);
	CNTRL_PORT &= ~(1<<RS_PIN);
	CNTRL_PORT |= (1<<ENABLE_PIN);
	_delay_us(2);
	CNTRL_PORT &= ~(1<<ENABLE_PIN);
	delay_us(100);
}

void LCD_send_data (unsigned char data) {
	DATA_PORT = data;
	CNTRL_PORT &= ~(1<<RW_PIN);
	CNTRL_PORT |= (1<<RS_PIN);

	CNTRL_PORT |= (1<<ENABLE_PIN);
	_delay_us(2);
	CNTRL_PORT &= ~(1<<ENABLE_PIN);
	delay_us(100);
}

void LCD_goto(unsigned char y, unsigned char x) {
	unsigned char firstAddress[] = [0x90, 0xC0, 0x94, 0xD4];

	LCD_send_command(firstAddress[y-1] + x-1);
	_delay_ms(10);
}

void LCD_print(char* string) {
	while(*string > 0) {
		LCD_send_data(*string++);
	}
}

void LCD_blink() {
	LCD_send_command(0x08);
	_delay_ms(250);
	LCD_send_command(0x0C);
	_delay_ms(250);
}

void LCD_clear(void) {
	LCD_send_command(0x01);
	_delay_ms(100);
}