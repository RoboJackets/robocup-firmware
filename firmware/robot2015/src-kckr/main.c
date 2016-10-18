#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "kicker_commands.h"
#include "pins.h"

#define NO_COMMAND 0

#define TIMING_CONSTANT 125
#define VOLTAGE_READ_DELAY_MS 40
#define VOLTAGE_CUTOFF 100

// Used to time kick and chip durations
volatile unsigned millis_left_ = 0;

// Used to keep track of current button state
/* volatile int kick_db_held_down_ = 0; */
/* volatile int chip_db_down_ = 0; */
/* volatile int charge_db_down_ = 0; */

volatile uint8_t byte_cnt = 0;

volatile uint8_t cur_command_ = NO_COMMAND;

// always up-to-date voltage so we don't have to get_voltage() inside interupts
volatile uint8_t last_voltage_ = 0;

volatile bool charge_allowed_ = false;

// executes a command coming from SPI
uint8_t execute_cmd(uint8_t, uint8_t);

/* Voltage Function */
uint8_t get_voltage() {
    // Start conversation by writing to start bit
    ADCSRA |= _BV(ADSC);

    // Wait for ADSC bit to clear
    while (ADCSRA & _BV(ADSC))
        ;

    // ADHC will go from 0 to 255 corresponding to 0 through VCC
    return ADCH;
}

/*
 * Returns true if charging is currently active
 */
bool is_charging() { return PORTA & _BV(CHARGE_PIN); }

/*
 * Returns true if the chip's SPI slave interface is currently
 * being selected by a master device.
 */
bool is_chip_selected() { return !(PINA & _BV(N_KICK_CS_PIN)); }

void main() {
    // make sure we're not kicking/chipping right off the start
    PORTA &= ~(_BV(KICK_PIN) | _BV(CHIP_PIN));

    // ensure KICK_PIN, CHIP_PIN, and CHARGE_PIN are outputs
    DDRA |= _BV(KICK_PIN) | _BV(CHIP_PIN) |
            _BV(CHARGE_PIN);  // MISO is handled by CS interrupt

    // ensure N_KICK_CS & MISO are inputs
    /* DDRA &= ~(_BV(N_KICK_CS_PIN) | _BV(KCKR_MISO_PIN)); */

    // when DDRB = 0 and PORTB = 1, these are configured as pull-up inputs,
    // when a button is pressed, these pins are driven towards ground

    // PORTB refers to pull-up or not
    /* PORTB |= _BV(DB_KICK_PIN) | _BV(DB_CHIP_PIN) | _BV(DB_CHG_PIN); */

    // ensure debug buttons are inputs
    /* DDRB &= ~_BV(DB_KICK_PIN) & ~_BV(DB_CHIP_PIN) & ~_BV(DB_CHG_PIN); */

    // enable interrupts for PCINT0-PCINT7
    GIMSK |= _BV(PCIE0);

    // enable interrupts for PCINT8-PCINT11
    GIMSK |= _BV(PCIE1);

    // only have the N_KICK_CS interrupt enabled
    PCMSK0 = _BV(INT_N_KICK_CS);

    // enable interrupts on debug buttons
    PCMSK1 = _BV(INT_DB_KICK) | _BV(INT_DB_CHIP) | _BV(INT_DB_CHG);

    // hard-coded for PA1, check datasheet before changing
    // Set lower three bits to value of pin we read from
    ADMUX |= V_MONITOR_PIN;

    // Interrupt on TIMER 0
    TIMSK0 |= _BV(OCIE0A);

    // CTC - Clear Timer on Compare Match
    TCCR0A |= _BV(WGM01);

    // OCR0A is max val of timer before reset
    // we need 1000 clocks at 1 Mhz to get 1 millisecond
    // if we prescale by 8, then we need 125 on timer to get 1 ms exactly
    OCR0A = TIMING_CONSTANT;  // reset every millisecond

    // ADC Initialization
    PRR &= ~_BV(PRADC);    // disable power reduction - Pg. 133
    ADCSRA |= _BV(ADEN);   // enable the ADC - Pg. 133
    ADCSRB |= _BV(ADLAR);  // present left adjusted
    // because we left adjusted and only need 8 bit precision,
    // we can now read ADCH directly

    // enable global interrupts
    sei();

    // needs to be int to force voltage_accum calculation to use ints
    const int kalpha = 32;

    // We handle voltage readings here
    while (true) {
        // get a voltage reading by weighing in a new reading, same concept as
        // TCP RTT estimates (exponentially weighted sum)
        last_voltage_ = get_voltage();
        int voltage_accum =
            (255 - kalpha) * last_voltage_ + kalpha * get_voltage();
        last_voltage_ = voltage_accum / 255;

        if (charge_allowed_ && last_voltage_ < VOLTAGE_CUTOFF) {
            PORTA |= _BV(CHARGE_PIN);
        } else {
            PORTA &= ~(_BV(CHARGE_PIN));
        }

        _delay_ms(VOLTAGE_READ_DELAY_MS);
    }
}

/*
 * SPI Start Interrupt, this should never get called when chip select is high.
 *
 * NOTE: This is intentionally not done with an overflow interrupt,
 * as the software chip select interrupt will be triggered almost at
 * the same time as an SPI overflow, causing the SPI interrupt to not be
 * handled.
 */
ISR(USI_STR_vect, ISR_BLOCK) {
    // wait for overflow flag to become 1
    while (!(USISR & _BV(USIOIF)))
        ;

    // get data from USIDR
    uint8_t recv_data = USIDR;

    // clear the SPI start flag
    USISR |= _BV(USISIF);
    // clear the overflow flag
    USISR |= _BV(USIOIF);

    // increment our received byte count and take appropiate action
    byte_cnt++;
    if (byte_cnt == 1) {
        // we don't have a command already, set the response
        // buffer to the command we received to let the
        // master confirm the given command if desired, top
        // bit is set if currently charging
        cur_command_ = recv_data;
        USIDR = cur_command_;
    } else if (byte_cnt == 2) {
        // execute the currently set command with
        // the newly given argument, set the response
        // buffer to our return value
        USIDR = execute_cmd(cur_command_, recv_data);
    } else {
        USIDR = 0x11;
    }
}

/*
 * Clears the USISR to remove any previous SPI state.
 */
void clear_spi_state() {
    USISR = 1 << USISIF | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |
            (0 << USICNT3) | (0 << USICNT2) | (0 << USICNT1) | (0 << USICNT0);
}

/*
 * Initializes the USICR for three wire SPI.
 */
void turn_on_spi() {
    USICR = (1 << USISIE)    // enable start condition interrupt enable
            | (0 << USIOIE)  // disable overflow interrupt for now
            | (0 << USIWM1)  // set to three wire mode (normal SPI)
            | (1 << USIWM0) |
            (1 << USICS1)    // next three bits define how clock works
            | (0 << USICS0)  // counter triggered both edge
            | (0 << USICLK)  // USIDR shifts on positive edge only
            | (0 << USITC);
}

/*
 * Disables SPI.
 */
void turn_off_spi() { USICR = 0; }

/*
 * Chip Select Interrupt
 * Turns on SPI when we get selected, and turns off SPI when
 * deselected.
 *
 * ISR for PCINT0 - PCINT7
 */
ISR(PCINT0_vect) {
    cur_command_ = NO_COMMAND;
    byte_cnt = 0;

    if (is_chip_selected()) {
        // set the slave data out pin as an output
        turn_on_spi();
        clear_spi_state();
        DDRA |= _BV(KCKR_MISO_PIN);
    } else {
        // set the slave data out pin as an input
        turn_off_spi();
        DDRA &= ~_BV(KCKR_MISO_PIN);

        if (is_charging()) {
            USIDR = ISCHARGING;
        } else {
            USIDR = NOTCHARGING;
        }
    }
}

/*
 * NOTE: Debug button functionality has been removed until we finalize
 * the hardware.
 * Interrupt if the state of any button has changed
 * Every time a button goes from LOW to HIGH, we will execute a command
 *
 * ISR for PCINT8 - PCINT11
 */
/* ISR(PCINT1_vect) { */
/*     // First we get the current state of each button */
/*     int kick_db_pressed = PINB & _BV(DB_KICK_PIN); */
/*     int chip_db_pressed = PINB & _BV(DB_CHIP_PIN); */
/*     int charge_db_pressed = PINB & _BV(DB_CHG_PIN); */

/*     // We only will execute commands when the user initially presses the */
/*     button */
/*     // So the old button state needs to be LOW and the new button state needs
 */
/*     // to be HIGH */
/*     if (!kick_db_held_down_ && kick_db_pressed) */
/*         execute_cmd(KICK_CMD, DB_KICK_TIME); */

/*     if (!chip_db_down_ && chip_db_pressed) execute_cmd(CHIP_CMD, */
/*     DB_CHIP_TIME); */

/*     // toggle charge */
/*     if (!charge_db_down_ && charge_db_pressed) { */
/*         // check if charge is on */
/*         if (PINA & _BV(CHARGE_PIN)) { */
/*             execute_cmd(SET_CHARGE_CMD, OFF_ARG); */
/*         } else { */
/*             execute_cmd(SET_CHARGE_CMD, ON_ARG); */
/*         } */
/*     } */

/*     // Now our last state becomes the current state of the buttons */
/*     kick_db_held_down_ = kick_db_pressed; */
/*     chip_db_down_ = chip_db_pressed; */
/*     charge_db_down_ = charge_db_pressed; */
/* } */

/*
 * Timer interrupt for chipping/kicking - called every millisecond by timer
 *
 * ISR for TIMER 0
 */
ISR(TIM0_COMPA_vect) {
    // decrement ms counter
    millis_left_--;

    // if the counter hits 0, clear the kick/chip pin state
    if (!millis_left_) {
        // could be kicking or chipping, clear both
        PORTA &= ~(_BV(KICK_PIN) | _BV(CHIP_PIN));
        // stop prescaled timer
        TCCR0B &= ~_BV(CS01);
    }
}

/*
 * Executes a command that can come from SPI or a debug button
 *
 * WARNING: This will be called from an interrupt service routines, keep it
 * short!
 */
uint8_t execute_cmd(uint8_t cmd, uint8_t arg) {
    // if we don't change ret_val by setting it to voltage or
    // something, then we'll just return the command we got as
    // an acknowledgement.
    uint8_t ret_val = BLANK;

    switch (cmd) {
        case KICK_CMD:
            millis_left_ = arg;
            PORTA |= _BV(KICK_PIN);  // set KICK pin
            TCCR0B |= _BV(CS01);     // start timer /8 prescale
            break;

        case CHIP_CMD:
            millis_left_ = arg;
            PORTA |= _BV(CHIP_PIN);  // set CHIP pin
            TCCR0B |= _BV(CS01);     // start timer /8 prescale
            break;

        case SET_CHARGE_CMD:
            // set state based on argument
            if (arg == ON_ARG) {
                charge_allowed_ = true;
            } else if (arg == OFF_ARG) {
                charge_allowed_ = false;
            }
            break;

        case GET_VOLTAGE_CMD:
            ret_val = last_voltage_;
            break;

        case PING_CMD:
            // do nothing, ping is just a way to check if the kicker
            // is connected by checking the returned command ack from
            // earlier.
            break;

        default:
            ret_val =
                0xCC;  // return a weird value to show arg wasn't recognized
            break;
    }

    return ret_val;
}
