#include <stdbool.h>

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "kicker_commands.h"
#include "pins.h"

// kicker parameters
#define MAX_KICK_STRENGTH 255.0f
#define MIN_EFFECTIVE_KICK_FET_EN_TIME 0.8f
#define MAX_EFFECTIVE_KICK_FET_EN_TIME 10.0f

#define KICK_TIME_SLOPE ((MAX_EFFECTIVE_KICK_FET_EN_TIME - MIN_EFFECTIVE_KICK_FET_EN_TIME)) / MAX_KICK_STRENGTH

#define KICK_COOLDOWN_MS 2000
#define PRE_KICK_SAFETY_MARGIN_MS 5
#define POST_KICK_SAFETY_MARGIN_MS 5

#define NO_COMMAND 0

#define TIMING_CONSTANT (10-1)
#define VOLTAGE_READ_DELAY_MS 40

// 8 Mhz clock ~ 0.125 us period
// timer CKDIV8 1 Mhz ~ 1 us period
// timer CMP value divides by 10, 0.1 Mhz ~ 10 us period
// 1 ms / 10 us = 100 timer interrupts per ms
#define MS_TO_TIMER 100

// get different ball reading for 20 * 100 us = 2 ms before switching
#define BALL_SENSE_MAX_SAMPLES 5

// Used to time kick and chip durations
volatile int32_t pre_kick_cooldown_ = 0;
volatile int32_t timer_cnts_left_ = 0;
volatile int32_t post_kick_cooldown_ = 0;
volatile int32_t kick_wait_ = 0;

// Used to keep track of current button state
volatile int kick_db_down_ = 0;
volatile int chip_db_down_ = 0;
volatile int charge_db_down_ = 0;

volatile uint8_t byte_cnt = 0;

volatile uint8_t cur_command_ = NO_COMMAND;

// always up-to-date voltage so we don't have to get_voltage() inside interupts
volatile uint8_t last_voltage_ = 0;

volatile bool ball_sensed_ = 0;

// whether or not MBED has requested charging to be on
volatile bool charge_commanded_ = false;

volatile bool charge_allowed_ = true;

volatile bool kick_on_breakbeam_ = false;
volatile uint8_t kick_on_breakbeam_strength_ = 0;

unsigned ball_sense_change_count_ = 0;

uint32_t time = 0;

// executes a command coming from SPI
uint8_t execute_cmd(uint8_t, uint8_t);

/*
 * Checks and returns if we're in the middle of a kick
 */
bool is_kicking() {
    return pre_kick_cooldown_ >= 0
            || timer_cnts_left_ >= 0
            || post_kick_cooldown_ >= 0 
            || kick_wait_ >= 0;
}

/*
 * start the kick FSM for desired strength. If the FSM is already running,
 * the call will be ignored.
 */
void kick(uint8_t strength) {
    // check if the kick FSM is running
    if (is_kicking()) return;

    // initialize the countdowns for pre and post kick
    pre_kick_cooldown_ = (PRE_KICK_SAFETY_MARGIN_MS * MS_TO_TIMER);
    post_kick_cooldown_ = (POST_KICK_SAFETY_MARGIN_MS * MS_TO_TIMER);
    kick_wait_ = ((int32_t) KICK_COOLDOWN_MS) * MS_TO_TIMER;

    // compute time the solenoid FET is turned on, in milliseconds, based on
    // min and max effective FET enabled times
    float strength_ratio = (strength / MAX_KICK_STRENGTH);
    float time_cnt_flt_ms = KICK_TIME_SLOPE * strength_ratio + MIN_EFFECTIVE_KICK_FET_EN_TIME;
    float time_cnt_flt = time_cnt_flt_ms * MS_TO_TIMER;
    timer_cnts_left_ = (int)(time_cnt_flt + 0.5f);  // round

    pre_kick_cooldown_ = 100;
    post_kick_cooldown_ = 100;
    kick_wait_ = 100;
    timer_cnts_left_ = 2 * MS_TO_TIMER;

    // start timer to enable the kick FSM processing interrupt
    TCCR0B |= _BV(CS01);
}

void init();

/* Voltage Function */
uint8_t get_voltage() {
    // Start conversation by writing to start bit
    ADCSRA |= _BV(ADSC);

    // Wait for ADSC bit to clear
    while (ADCSRA & _BV(ADSC))
        ;

    // ADHC will range from 0 to 255 corresponding to 0 through VCC
    return ADCH;
}

void main() {
    init();

    // needs to be int to force voltage_accum calculation to use ints
    const int kalpha = 32;

    // We handle voltage readings here
    while (true) {
        // get a voltage reading by weighing in a new reading, same concept as
        // TCP RTT estimates (exponentially weighted sum)

        // 10 us * 4000 = 40 ms, we don't need to run the ADC much faster
        if (time % 4000 == 0) {
            int voltage_accum =
                (255 - kalpha) * last_voltage_ + kalpha * get_voltage();
            last_voltage_ = voltage_accum / 255;
        }

        // if we dropped below acceptable voltage, then this will catch it
        // note: these aren't true voltages, just ADC output, but it matches fairly close
        if (last_voltage_ > 239 || !charge_allowed_ || !charge_commanded_) {
            PORTB &= ~(_BV(CHARGE_PIN));
        } else if (last_voltage_ < 232 && charge_allowed_ &&
                   charge_commanded_) {
            PORTB |= _BV(CHARGE_PIN);
        }

        if (PINA & _BV(N_KICK_CS_PIN)) {
            byte_cnt = 0;
        }

        bool bs = PINB & _BV(BALL_SENSE_RX);
        if (ball_sensed_) {
            if (!bs)
                ball_sense_change_count_++;  // wrong reading, inc counter
            else
                ball_sense_change_count_ = 0;  // correct reading, reset counter
        } else {
            if (bs)
                ball_sense_change_count_++;  // wrong reading, inc counter
            else
                ball_sense_change_count_ = 0;  // correct reading, reset counter
        }

        // counter exceeds maximium, so reset
        if (ball_sense_change_count_ > BALL_SENSE_MAX_SAMPLES) {
            ball_sense_change_count_ = 0;

            ball_sensed_ = !ball_sensed_;
        }

        if (ball_sensed_ && kick_on_breakbeam_) {
            // pow
            kick(kick_on_breakbeam_strength_);
            kick_on_breakbeam_ = false;
        }

        _delay_us(10);
    }
}

void init() {
    // disable interrupts
    cli();

    // disable watchdog
    wdt_reset();
    MCUSR &= ~(_BV(WDRF));
    WDTCR |= (_BV(WDCE)) | (_BV(WDE));
    WDTCR = 0x00;

    // change our clock speed from 1 Mhz to 8 Mhz (there's a default CLKDIV8)
    // 1. write the Clock Prescaler Change Enable (CLKPCE) bit to one and all
    //    other bits in CLKPR to zero.
    // 2. within four cycles, write the desired value to CLKPS while writing a
    //    zero to CLKPCE.
    CLKPR = _BV(CLKPCE);
    CLKPR = 0; // corresponds to CLKDIV1 prescale, also keeps CLKPCE low

    // configure output pins
    DDRA |= _BV(KICK_MISO_PIN);
    DDRB |= _BV(KICK_PIN) | _BV(CHARGE_PIN) | _BV(BALL_SENSE_TX);

    // enable ball sense LED
    PORTB |= _BV(BALL_SENSE_TX);

    // configure input pins
    DDRA &= ~(_BV(N_KICK_CS_PIN) | _BV(V_MONITOR_PIN) | _BV(KICK_MOSI_PIN));

    PORTB &= ~(_BV(BALL_SENSE_RX));
    DDRB &= ~(_BV(BALL_SENSE_RX));

    // configure SPI
    SPCR = _BV(SPE) | _BV(SPIE);
    SPCR &= ~(_BV(MSTR));  // ensure we are a slave SPI device

    // enable interrupts for PCINT0-PCINT7
    PCICR |= _BV(PCIE0);

    // enable interrupts on debug buttons
    PCMSK0 = _BV(INT_DB_KICK) | _BV(INT_DB_CHG);

    ///////////////////////////////////////////////////////////////////////////
    //  TIMER INITIALIZATION
    //
    //  The timer works by interrupt callback. The timer is based off of an
    //  accumulator register that is incremented per clock tick. When the
    //  accumulator register reaches the value in the target register,
    //  the interrupt fires.
    //
    //  Initialization
    //  1) The interrupt enable bit is set in TIMSK0
    //  2) Clear the bit indicating timer matched the target/compare register
    //  3) Set the value of the target/compare register
    //
    //  Callback
    //  ISR(TIMER0_COMPA_vect)
    //
    //  Start/Global
    //  kick()
    //
    //  initialize timer
    TIMSK0 |= _BV(OCIE0A);    // Interrupt on TIMER 0
    TCCR0A |= _BV(WGM01);     // CTC - Clear Timer on Compare Match
    OCR0A = TIMING_CONSTANT;  // OCR0A is max val of timer before reset
    ///////////////////////////////////////////////////////////////////////////

    // Set low bits corresponding to pin we read from
    ADMUX |= _BV(ADLAR) | 0x00;  // connect PA0 (V_MONITOR_PIN) to ADC

    // ensure ADC isn't shut off
    PRR &= ~_BV(PRADC);
    ADCSRA |= _BV(ADEN);  // enable the ADC - Pg. 133

    // enable global interrupts
    sei();
}

/*
 * SPI Interrupt. Triggers when we have a new byte available, it'll be
 * stored in SPDR. Writing a response also occurs using the SPDR register.
 */
ISR(SPI_STC_vect) {
    uint8_t recv_data = SPDR;

    SPDR = 0xFF;
    // increment our received byte count and take appropriate action
    if (byte_cnt == 0) {
        cur_command_ = recv_data;
        // kicker status fields
        SPDR |= ACK;
    } else if (byte_cnt == 1) {
        // execute the currently set command with
        // the newly given argument, set the response
        // buffer to our return value
        SPDR = execute_cmd(cur_command_, recv_data);
    } else if (byte_cnt == 2) {
        SPDR = ((ball_sensed_ ? 1 : 0) << BALL_SENSE_FIELD) |
               ((charge_commanded_ ? 1 : 0) << CHARGE_FIELD) |
               ((kick_on_breakbeam_ ? 1 : 0) << KICK_ON_BREAKBEAM_FIELD) |
               ((is_kicking() ? 1 : 0) << KICKING_FIELD);
    } else if (byte_cnt == 4) {
        // no-op
    }
    int NUM_BYTES = 4;
    byte_cnt++;
    byte_cnt %= NUM_BYTES;
}

/*
 * Interrupt if the state of any button has changed
 * Every time a button goes from LOW to HIGH, we will execute a command
 *
 * ISR for PCINT8 - PCINT11
 */
ISR(PCINT0_vect) {
    // First we get the current state of each button, active low
    int dbg_switched = !(PINB & _BV(DB_SWITCH));

    if (!dbg_switched) return;
    int kick_db_pressed = !(PINA & _BV(DB_KICK_PIN));
    int charge_db_pressed = !(PINA & _BV(DB_CHG_PIN));

    if (!kick_db_down_ && kick_db_pressed) kick(255);

    // toggle charge
    if (!charge_db_down_ && charge_db_pressed) {
        // check if charge is already on, toggle appropriately
        charge_commanded_ = !charge_commanded_;
    }

    // Now our last state becomes the current state of the buttons
    kick_db_down_ = kick_db_pressed;
    charge_db_down_ = charge_db_pressed;
}

/*
 * Timer interrupt for chipping/kicking - called every millisecond by timer
 *
 * ISR for TIMER 0
 *
 * Pre and post cool downs add time between kicking and charging
 *
 * Charging while kicking is destructive to the charging circuitry
 * If no outstanding coutners are running from the timer, the pre, active, post,
 *and cooldown
 * states are all finished. We disable the timer to avoid unnecessary ISR
 *invocations when
 * there's nothing to do. The kick function will reinstate the timer.
 *
 * TCCR0B:
 * CS00 bit stays at zero
 * When CS01 is also zero, the clk is diabled
 * When CS01 is one, the clk is prescaled by 8
 * (When CS00 is one, and CS01 is 0, no prescale. We don't use this)
 */
ISR(TIMER0_COMPA_vect) {
    sei();
    if (pre_kick_cooldown_ >= 0) {
        /* PRE KICKING STATE
         * stop charging
         * wait between stopping charging and kicking for safety
         */

        // disable charging
        charge_allowed_ = false;

        pre_kick_cooldown_--;
    } else if (timer_cnts_left_ >= 0) {
        /* KICKING STATE
         * assert the kick pin, enabling the kick FET
         * wait for kick interval to end
         */

        // set KICK pin
        PORTB |= _BV(KICK_PIN);

        timer_cnts_left_--;
    } else if (post_kick_cooldown_ >= 0) {
        /* POST KICKING STATE
         * deassert the kick pin, disabling the kick FET
         * wait between stopping the FET and reenabling charging in the next
         * state
         */

        // kick is done
        PORTB &= ~_BV(KICK_PIN);

        post_kick_cooldown_--;
    } else if (kick_wait_ >= 0) {
        /* POST KICK COOLDOWN
         * enable charging
         * don't allow kicking during the cooldown
         */

        // reenable charching
        charge_allowed_ = true;

        kick_wait_--;
    } else {
        /* IDLE/NOT RUNNING
         * stop timer
         */

        // stop prescaled timer
        TCCR0B &= ~_BV(CS01);
    }
    cli();
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
        case KICK_BREAKBEAM_CMD:
            kick_on_breakbeam_ = true;
            kick_on_breakbeam_strength_ = arg;
            break;

        case KICK_BREAKBEAM_CANCEL_CMD:
            kick_on_breakbeam_ = false;
            kick_on_breakbeam_strength_ = 0;
            break;

        case KICK_IMMEDIATE_CMD:
            kick(arg);
            break;

        case SET_CHARGE_CMD:
            // set state based on argument
            if (arg == ON_ARG) {
                ret_val = 1;
                charge_commanded_ = true;
            } else if (arg == OFF_ARG) {
                ret_val = 0;
                charge_commanded_ = false;
            }
            break;

        case GET_VOLTAGE_CMD:
            ret_val = last_voltage_;
            break;

        case PING_CMD:
            ret_val = 0xFF;
            // do nothing, ping is just a way to check if the kicker
            // is connected by checking the returned command ack from
            // earlier.
            break;

        default:
            // return error value to show arg wasn't recognized
            ret_val = 0xCC;
            break;
    }

    return ret_val;
}
