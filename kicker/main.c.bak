#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <stdlib.h>
#include <util/delay.h>

#include "kicker_commands.h"
#include "kicker_config.h"
#include "pins.h"
#include "util.h"

// kicker parameters
#define MAX_KICK_STRENGTH 255.0f
#define MIN_EFFECTIVE_KICK_FET_EN_TIME 0.8f
#define MAX_EFFECTIVE_KICK_FET_EN_TIME 10.0f

#define KICK_TIME_SLOPE                                                        \
  (MAX_EFFECTIVE_KICK_FET_EN_TIME - MIN_EFFECTIVE_KICK_FET_EN_TIME)

#define KICK_COOLDOWN_MS 2000
#define PRE_KICK_SAFETY_MARGIN_MS 5
#define POST_KICK_SAFETY_MARGIN_MS 5

#define NO_COMMAND 0

// number of steps of resolution we want per millisecond
#define TIMER_PER_MS 4

// calculate our TIMING_CONSTANT (timer cmp val) from the desired resolution
#define CLK_FREQ 8000000 // after removing default CLKDIV8 prescale
#define TIMER_PRESCALE                                                         \
  8 // set by TCCR0B |= _BV(CS01) which also starts the timer
#define MS_PER_SECOND 1000

#define MAX_TIMER_FREQ (CLK_FREQ / TIMER_PRESCALE)
#define DESIRED_TIMER_FREQ (TIMER_PER_MS * MS_PER_SECOND)

#define TIMING_CONSTANT ((MAX_TIMER_FREQ / DESIRED_TIMER_FREQ) - 1)

// get different ball reading for 20 * 100 us = 2 ms before switching
#define BALL_SENSE_MAX_SAMPLES 5

// Used to time kick and chip durations, -1 indicates inactive state
volatile int32_t pre_kick_cooldown_ = -1;
volatile int32_t timer_cnts_left_ = -1;
volatile int32_t post_kick_cooldown_ = -1;
volatile int32_t kick_wait_ = -1;

//Check for chip type
volatile bool kick_type_is_chip_ = false;
volatile bool kick_type_is_chip_command_ = false;

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

volatile bool _in_debug_mode = false;

unsigned ball_sense_change_count_ = 0;

uint32_t time = 0;

// executes a command coming from SPI
uint8_t execute_cmd(uint8_t);

/*
 * Checks and returns if we're in the middle of a kick
 */
bool is_kicking() {
  return pre_kick_cooldown_ >= 0 || timer_cnts_left_ >= 0 ||
         post_kick_cooldown_ >= 0 || kick_wait_ >= 0;
}

/*
 * start the kick FSM for desired strength. If the FSM is already running,
 * the call will be ignored.
 */
void kick(uint8_t strength, bool is_chip) {

  // check if the kick FSM is running
  if (is_kicking())
    return;

  // initialize the countdowns for pre and post kick
  pre_kick_cooldown_ = (PRE_KICK_SAFETY_MARGIN_MS * TIMER_PER_MS);
  post_kick_cooldown_ = (POST_KICK_SAFETY_MARGIN_MS * TIMER_PER_MS);
  // force to int32_t, default word size too small
  kick_wait_ = ((int32_t)KICK_COOLDOWN_MS) * TIMER_PER_MS;

  // compute time the solenoid FET is turned on, in milliseconds, based on
  // min and max effective FET enabled times
  float strength_ratio = (strength / MAX_KICK_STRENGTH);
  float time_cnt_flt_ms =
      KICK_TIME_SLOPE * strength_ratio + MIN_EFFECTIVE_KICK_FET_EN_TIME;
  float time_cnt_flt = time_cnt_flt_ms * TIMER_PER_MS;
  timer_cnts_left_ = (int)(time_cnt_flt + 0.5f); // round

  kick_type_is_chip_ = is_chip;

  // start timer to enable the kick FSM processing interrupt
  TCCR0 |= _BV(CS00);
  // TCCR0 |= 0b010;
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
  const int kalpha = 64;

  // We handle voltage readings here
  while (true) {

    if (_in_debug_mode) {
      /* PORTC &= ~(_BV(MCU_YELLOW)); */
      char kick_db_pressed = !(PIND & _BV(DB_KICK_PIN));
      char charge_db_pressed = !(PIND & _BV(DB_CHG_PIN));
      char chip_db_pressed = !(PINB & _BV(DB_CHIP_PIN));

      /* PORTC |= _BV(MCU_RED); */
      /* PORTC &= ~(charge_db_pressed ? _BV(MCU_RED) : 0); */

      if (!kick_db_down_ && kick_db_pressed)
        kick(255, false);

      if(!chip_db_down_ && chip_db_pressed)
        kick(255, true);

      if (!charge_db_down_ && charge_db_pressed)
        charge_commanded_ = !charge_commanded_;

      kick_db_down_ = kick_db_pressed;
      chip_db_down_ = chip_db_pressed;
      charge_db_down_ = charge_db_pressed;
    } else {
      /* PORTC &= ~(_BV(MCU_YELLOW)); */
    }

    if (PINA & _BV(BALL_SENSE_RX))
      PORTB &= ~(_BV(BALL_SENSE_LED));
    else
      PORTB |= _BV(BALL_SENSE_LED);

    if (is_kicking())
      PORTC &= ~(_BV(MCU_YELLOW));
    else
      PORTC |= _BV(MCU_YELLOW);
    // get a voltage reading by weighing in a new reading, same concept as
    // TCP RTT estimates (exponentially weighted sum)

    // don't run the adc every loop
    if (time % 1000 == 0) {
      int voltage_accum =
          (255 - kalpha) * last_voltage_ + kalpha * get_voltage();
      last_voltage_ = voltage_accum / 255;

      int num_lights = ((int) last_voltage_ / 47);

      PORTA |= (0x1F << 1);
      PORTA &= ~((0xFF >> abs(num_lights - (sizeof(unsigned char) * 8))) << 1);
    }
    time++;

    // if we dropped below acceptable voltage, then this will catch it
    // note: these aren't true voltages, just ADC output, but it matches
    // fairly close

    if (!(PIND & _BV(LT_DONE_N)) || last_voltage_ > 239 || !charge_allowed_ ||
        !charge_commanded_) {
      PORTD &= ~(_BV(LT_CHARGE));
    } else if (last_voltage_ < 232 && charge_allowed_ && charge_commanded_) {
      PORTD |= _BV(LT_CHARGE);
    }

    if (PINB & _BV(N_KICK_CS_PIN)) {
      byte_cnt = 0;
    }

    bool bs = PINA & _BV(BALL_SENSE_RX);
    if (ball_sensed_) {
      if (!bs)
        ball_sense_change_count_++; // wrong reading, inc counter
      else
        ball_sense_change_count_ = 0; // correct reading, reset counter
    } else {
      if (bs)
        ball_sense_change_count_++; // wrong reading, inc counter
      else
        ball_sense_change_count_ = 0; // correct reading, reset counter
    }

    // counter exceeds maximium, so reset
    if (ball_sense_change_count_ > BALL_SENSE_MAX_SAMPLES) {
      ball_sense_change_count_ = 0;

      ball_sensed_ = !ball_sensed_;
    }

    if (ball_sensed_ && kick_on_breakbeam_) {
      // pow
      kick(kick_on_breakbeam_strength_,false);
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
  WDTCR |= (_BV(WDTOE) | _BV(WDE));
  WDTCR = 0x00;

  SFIOR |= _BV(PUD);

  // config status LEDs and set ERR/WARN until init done
  DDRD |= _BV(MCU_GREEN);
  DDRC |= _BV(MCU_YELLOW);
  DDRC |= _BV(MCU_RED);

  PORTC |= _BV(MCU_YELLOW);
  PORTC |= _BV(MCU_RED);

  DDRB |= _BV(CHIP_PIN);

  // latch debug state
  _in_debug_mode = (PINC & _BV(DB_SWITCH));

  // configure core io
  DDRB |= _BV(KICK_MISO_PIN);
  DDRB &= ~(_BV(N_KICK_CS_PIN) | _BV(KICK_MOSI_PIN) | _BV(KICK_CLK_PIN));

  // configure hv mon
  DDRA &= ~(_BV(V_MONITOR_PIN));
  DDRA |= (_BV(HV_IND_MAX) | _BV(HV_IND_HIGH) | _BV(HV_IND_MID) |
           _BV(HV_IND_LOW) | _BV(HV_IND_MIN));

  // configure LT3751
  DDRD |= _BV(LT_CHARGE);
  DDRD &= ~(_BV(LT_DONE_N) | _BV(LT_FAULT_N));

  // configure ball sense
  DDRD |= (_BV(BALL_SENSE_TX));
  DDRB |= _BV(BALL_SENSE_LED);
  // PORTD &= ~(_BV(BALL_SENSE_TX));
  PORTB |= _BV(BALL_SENSE_LED);
  PORTD |= _BV(BALL_SENSE_TX);
  DDRA &= ~(_BV(BALL_SENSE_RX));

  // configure debug
  DDRC &= ~(_BV(DB_SWITCH));
  DDRD &=
      ~(_BV(DB_CHG_PIN) | _BV(DB_KICK_PIN));
  DDRB |= (_BV(DB_CHIP_PIN));

  // disable JTAG
  MCUCSR |= (1 << JTD);
  MCUCSR |= (1 << JTD);
  // configure SPI
  if (!_in_debug_mode) {
      SPCR = _BV(SPE) | _BV(SPIE);
      SPCR &= ~(_BV(MSTR));
  }

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
  TIMSK |= _BV(OCIE0);    // Interrupt on TIMER 0
  // TCCR0 |= _BV(COM01);    // COM01 - Clear Timer on Compare Match
  OCR0 = TIMING_CONSTANT; // OCR0A is max val of timer before reset
  ///////////////////////////////////////////////////////////////////////////

  // Set low bits corresponding to pin we read from
  ADMUX |= _BV(ADLAR) | 0x06; // connect PA6 (V_MONITOR_PIN) to ADC

  // ensure ADC isn't shut off
  // PRR &= ~_BV(PRADC);
  ADCSRA |= _BV(ADEN); // enable the ADC - Pg. 133

  // enable global interrupts
  sei();
}

/*
 * SPI Interrupt. Triggers when we have a new byte available, it'll be
 * stored in SPDR. Writing a response also occurs using the SPDR register.
 */
ISR(SPI_STC_vect) {
  SPDR = execute_cmd(SPDR);
}

/*
 * Interrupt if the state of any button has changed
 * Every time a button goes from LOW to HIGH, we will execute a command
 *
 * ISR for PCINT8 - PCINT11
 */
/*
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
*/

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
ISR(TIMER0_COMP_vect) {
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
    PORTC &= ~(_BV(MCU_RED));
    if (kick_type_is_chip_) {
      PORTB |= _BV(CHIP_PIN);
    } else {
      PORTB |= _BV(KICK_PIN);
    }

    timer_cnts_left_--;
  } else if (post_kick_cooldown_ >= 0) {
    /* POST KICKING STATE
     * deassert the kick pin, disabling the kick FET
     * wait between stopping the FET and reenabling charging in the next
     * state
     */

    // kick is done
    PORTC |= _BV(MCU_RED);
    if (kick_type_is_chip_) {
      PORTB &= ~_BV(CHIP_PIN);
    } else {
      PORTB &= ~_BV(KICK_PIN);
    }

    post_kick_cooldown_--;
  } else if (kick_wait_ >= 0) {
    /* POST KICK COOLDOWN
     * enable charging
     * don't allow kicking during the cooldown
     */

    // reenable charging
    charge_allowed_ = true;

    kick_wait_--;
  } else {
    /* IDLE/NOT RUNNING
     * stop timer
     */

    // stop prescaled timer
    TCCR0 &= ~_BV(CS00);
    // TCCR0 &= 0b00;
  }
}

/*
 * Executes a command that can come from SPI or a debug button
 *
 * WARNING: This will be called from an interrupt service routines, keep it
 * short!
 */
uint8_t execute_cmd(uint8_t cmd) {
  // if we don't change ret_val by setting it to voltage or
  // something, then we'll just return the command we got as
  // an acknowledgement.
  bool allow_charge = !!(cmd & (1 << 4));
  uint8_t kick_power = (cmd & 0xF) << 4;
  uint8_t kick_activation = cmd & (3 << 5);
  bool use_chip = !!(cmd & (1 << 7));

  if (allow_charge) {
    charge_commanded_ = true;
  } else {
    charge_commanded_ = false;
  }

  if (kick_activation == KICK_ON_BREAKBEAM) {
    kick_on_breakbeam_ = true;
    kick_on_breakbeam_strength_ = kick_power;
  } else if (kick_activation == KICK_IMMEDIATE) {
    kick(kick_power,false);
  } else if (kick_activation == CANCEL_KICK) {
    kick_on_breakbeam_ = false;
    kick_on_breakbeam_strength_ = 0;
  }

  return (ball_sensed_ << 7) | (VOLTAGE_MASK & (last_voltage_ >> 1));
}
