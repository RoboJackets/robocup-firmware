#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "kicker.h"

void init() {
    // disable interrupts
    cli();

    // disable watchdog
    wdt_reset();
    WDTCR |= (_BV(WDTOE) | _BV(WDE));
    WDTCR = 0x00;

    SFIOR |= _BV(PUD);

    // config status LEDs and set ERR/WARN until init done
    DDRC |= _BV(MCU_YELLOW);
    DDRC |= _BV(MCU_RED);

    // configure debug
    DDRC &= ~(_BV(DB_SWITCH));

    // enable global interrupts
    sei();
}

int main() {
    init();

    for (;;) {
    if (PINC & _BV(DB_SWITCH)) {
        PORTC &= _BV(MCU_YELLOW);
        PORTC &= _BV(MCU_RED);
    } else {
        PORTC |= _BV(MCU_YELLOW);
        PORTC |= _BV(MCU_RED);
    }
}
}
