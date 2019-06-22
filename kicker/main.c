#include <stdbool.h>

#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>
#include <stdlib.h>
#include <util/delay.h>

#include "HAL_atmega32a.h"

#include "kicker_config.h"
#include "kicker_commands.h"
#include "pins.h"
#include "util.h"

// kicker parameters
#define MAX_KICK_STRENGTH 15.0f //2^4 - 1
#define MIN_EFFECTIVE_KICK_FET_EN_TIME 0.8f
#define MAX_EFFECTIVE_KICK_FET_EN_TIME 10.0f

#define KICK_TIME_SLOPE \
    (MAX_EFFECTIVE_KICK_FET_EN_TIME - MIN_EFFECTIVE_KICK_FET_EN_TIME)

// How much time to give for the LT to stop charging the caps
#define STOP_CHARGING_SAFETY_MARGIN_MS 5
// How much time to give the FET to stop current flow
#define STOP_FLOW_SAFETY_MARGIN_MS 5
// Time after the kick to allow the caps to charge back up somewhat
#define CHARGE_TIME_MS 2000

// number of steps of resolution we want per millisecond
#define TIMER_PER_MS 4

// calculate our TIMING_CONSTANT (timer cmp val) from the desired resolution
#define CLK_FREQ 8000000  // after removing default CLKDIV8 prescale
#define TIMER_PRESCALE \
   8  // set by TCCR0B |= _BV(CS01) which also starts the timer
#define MS_PER_SECOND 1000

#define MAX_TIMER_FREQ (CLK_FREQ / TIMER_PRESCALE)
#define DESIRED_TIMER_FREQ (TIMER_PER_MS * MS_PER_SECOND)

#define TIMING_CONSTANT ((MAX_TIMER_FREQ / DESIRED_TIMER_FREQ) - 1)

// get different ball reading for X * 10 us before switching
#define BALL_SENSE_MAX_SAMPLES 5


// Used to time kick and chip durations, -1 indicates inactive state
volatile struct {
    int32_t stop_charge_phase; // Allow time for main loop to stop charge
    int32_t flow_phase;        // Let current flow during this phase
    int32_t stop_flow_phase;   // Allow time for current to stop flowing
    int32_t charge_phase;      // Allow time for caps to charge back up
} time = {-1, -1, -1, -1};

// Latest command from mtrain
volatile struct {
    bool kick_type_is_kick; // chip or kick?
    bool kick_immediate; // Kick immediately?
    bool kick_on_breakbeam; // Kick when breakbeam triggers?
    bool commanded_charge; // Commanded to charge the caps?
    uint8_t kick_power; // Commanded power to kick at
} command = {true, false, false, false, 0};

// Current kick command
volatile bool current_kick_type_is_kick = true;

// Global vars so we don't have to read pins in interrupts etc
volatile uint8_t current_voltage = 0;
volatile bool ball_sensed = false;

// Global state variables
volatile bool in_debug_mode = false;
volatile bool charge_allowed = true; // Don't charge during kick

void init();

/*
 * Checks and returns if we're in the middle of a kick
 */
bool is_kicking() {
    return time.stop_charge_phase >= 0 ||
           time.flow_phase >= 0 ||
           time.stop_flow_phase >= 0 ||
           time.charge_phase >= 0;
}

/**
 * start the kick FSM for desired strength. If the FSM is already running,
 * the call will be ignored.
 */
void kick(uint8_t strength, bool is_kick) {
    // check if the kick FSM is running
    if (is_kicking()) return;

    // initialize the countdowns for pre and post kick
    time.stop_charge_phase = (STOP_CHARGING_SAFETY_MARGIN_MS * TIMER_PER_MS);

    // compute time the solenoid FET is turned on, in milliseconds, based on
    // min and max effective FET enabled times
    float strength_ratio = (strength / MAX_KICK_STRENGTH);
    float time_cnt_flt_ms =
        KICK_TIME_SLOPE * strength_ratio + MIN_EFFECTIVE_KICK_FET_EN_TIME;
    float time_cnt_flt = time_cnt_flt_ms * TIMER_PER_MS;
    time.flow_phase = (int)(time_cnt_flt + 0.5f);  // round

    time.stop_flow_phase   = (STOP_FLOW_SAFETY_MARGIN_MS * TIMER_PER_MS);

    // force to int32_t, default word size too small
    time.charge_phase = ((int32_t)CHARGE_TIME_MS) * TIMER_PER_MS;

    // Set kick type after we have commited to the kick
    // such that it doesn't change halfway through the kick
    current_kick_type_is_kick = is_kick;

    // start timer to enable the kick FSM processing interrupt
    TCCR0 |= _BV(CS00) & 0b01;
}

void handle_debug_mode() {
    // Used to keep track of current button state
    static bool kick_db_down = false;
    static bool chip_db_down = false;
    static bool charge_db_down = false;

    if (in_debug_mode) {
        // Check if the buttons are pressed
        bool kick_db_pressed = !(HAL_IsSet(DB_KICK_PIN));
        bool chip_db_pressed = !(HAL_IsSet(DB_CHIP_PIN));
        bool charge_db_pressed = !(HAL_IsSet(DB_CHG_PIN));

        // Simple rising edge triggers
        if (!kick_db_down && kick_db_pressed) 
            kick(255, true);

        if (!chip_db_down && chip_db_pressed)
            kick(255, false);

        // If we should be charging
        if (!charge_db_down && charge_db_pressed)
            command.commanded_charge = true;

        kick_db_down = kick_db_pressed;
        chip_db_down = chip_db_pressed;
        charge_db_down = charge_db_pressed;
    }
}

void set_state_LEDs() {
    // Check ball sense
    // Turn on led if we see something
    HAL_SetLED(BALL_SENSE_LED, HAL_IsSet(BALL_SENSE_RX));

    // If we are kicking, turn on yellow
    HAL_SetLED(MCU_YELLOW, is_kicking());

    // If we are chipping, bring up the red led
    // for debug purposes
    // todo remove this
    HAL_SetLED(MCU_RED, !current_kick_type_is_kick);
}

uint8_t get_voltage() {
    // Start conversation by writing to start bit
    ADCSRA |= _BV(ADSC);

    // Wait for ADSC bit to clear
    while (ADCSRA & _BV(ADSC));

    // ADHC will range from 0 to 255 corresponding to 0 through VCC
    return ADCH;
}

void try_read_voltage() {
    static uint32_t time = 0;

    // Don't run the adc every loop
    // 1000 * 10 us = 10 ms
    if (time % 1000 == 0) {
        // needs to be int to force voltage_accum calculation to use ints
        const int kalpha = 192;

        // get a voltage reading by weighing in a new reading, same concept as
        // TCP RTT estimates (exponentially weighted sum)
        int voltage_accum = (255 - kalpha) * current_voltage +
                            kalpha * get_voltage();
        current_voltage = voltage_accum / 255;

        // 1 light on at 0-50
        // 2 light on at 51-101
        // 3 light on at 102-152
        // 4 light on at 153-203
        // 5 light on at 204-255
        // At 255, num lights = 6, but default case allows same
        // behavior
        uint8_t num_lights = ((uint8_t) current_voltage / 51) + 1;

        // Clear charge level led's
        HAL_SetLED(HV_IND_MIN,  false);
        HAL_SetLED(HV_IND_LOW,  false);
        HAL_SetLED(HV_IND_MID,  false);
        HAL_SetLED(HV_IND_HIGH, false);
        HAL_SetLED(HV_IND_MAX,  false);

        // Toggle them on as the reaches different levels
        switch (num_lights) {
        default:
        case 5:
            HAL_SetLED(HV_IND_MAX, true);
        case 4:
            HAL_SetLED(HV_IND_HIGH, true);
        case 3:
            HAL_SetLED(HV_IND_MID, true);
        case 2:
            HAL_SetLED(HV_IND_LOW, true);
        case 1:
            HAL_SetLED(HV_IND_MIN, true);
        }
    }
    time++;
}

void update_ball_sense() {
    static uint32_t ball_sense_change_count = 0;

    // Filter ball value
    // Want X amount in a row to be the same
    bool new_reading = HAL_IsSet(BALL_SENSE_RX);

    // If we sensed the ball, but don't think it's triggered
    // or we didn't sense the ball, and think it's triggered
    // AKA our estimate is wrong
    if (ball_sensed ^ new_reading)
        ball_sense_change_count++;
    else // else correct reading
        ball_sense_change_count = 0;

    // We got a wrong reading X times in a row
    // so we should swap
    if (ball_sense_change_count > BALL_SENSE_MAX_SAMPLES) {
        ball_sense_change_count = 0;

        ball_sensed = !ball_sensed;
    }
}

void charge_caps() {
    // if we dropped below acceptable voltage, then this will catch it
    // note: these aren't true voltages, just ADC output, but it matches
    // fairly close
    
    // Stop charging if we are at the voltage target
    if (!HAL_IsSet(LT_DONE_N) ||
        current_voltage > 239 ||
        !charge_allowed ||
        !command.commanded_charge) {

        HAL_ClearPin(LT_CHARGE);

    // Charge if we are too low
    } else if (current_voltage < 232 &&
               charge_allowed &&
               command.commanded_charge) {

        HAL_SetPin(LT_CHARGE);
    }
}

void fill_spi_return() {
    uint8_t ret_byte = 0x00;

    if (ball_sensed)
        ret_byte |= BREAKBEAM_TRIPPED;

    ret_byte |= VOLTAGE_MASK & (current_voltage >> 1);

    SPDR = ret_byte;
}

void main() {
    init();

    while (true) {

        handle_debug_mode();

        set_state_LEDs();

        try_read_voltage();

        update_ball_sense();

        charge_caps();

        // Kick on give command
        if ((command.kick_on_breakbeam && ball_sensed) ||
            command.kick_immediate) {

            // pow
            kick(command.kick_power, command.kick_type_is_kick);
            command.kick_immediate = false;
            command.kick_on_breakbeam = false;
        }

        fill_spi_return();

        _delay_us(10);
    }
}

/*
 * SPI Interrupt. Triggers when we have a new byte available, it'll be
 * stored in SPDR. Writing a response also occurs using the SPDR register.
 * 
 * Receive the command and set the global variables accordingly
 */
ISR(SPI_STC_vect) {
    // Don't take commands in debug mode
    if (in_debug_mode)
        return;

    uint8_t recv_data = SPDR;

    // Fill our globals with the commands
    command.kick_type_is_kick = recv_data & TYPE_KICK;
    command.commanded_charge  = recv_data & START_CHARGE;
    command.kick_power        = recv_data & KICK_POWER_MASK;

    // If we get a cancel kick command
    // Stop the kicks
    if (recv_data & CANCEL_KICK == CANCEL_KICK) {
        command.kick_immediate = false;
        command.kick_on_breakbeam = false;

    // Set the correct kick action
    } else if (recv_data & KICK_IMMEDIATE) {
        command.kick_immediate    = true;
        command.kick_on_breakbeam = false;
    } else if (recv_data & KICK_ON_BREAKBEAM) {
        command.kick_immediate    = false;
        command.kick_on_breakbeam = true;
    }
}

/**
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
    if (time.stop_charge_phase >= 0) {
        /**
         * PRE KICKING STATE
         * stop charging
         * wait between stopping charging and kicking for safety
         */
        // disable charging
        charge_allowed = false;

        time.stop_charge_phase--;
    } else if (time.flow_phase >= 0) {
        /**
         * KICKING STATE
         * assert the kick pin, enabling the kick FET
         * wait for kick interval to end
         */

        HAL_SetLED(MCU_RED, true);

        if (current_kick_type_is_kick) {
            HAL_SetPin(KICK_PIN);
        } else {
            HAL_SetPin(CHIP_PIN);
        }

        time.flow_phase--;
    } else if (time.stop_flow_phase >= 0) {
        /**
         * POST KICKING STATE
         * deassert the kick pin, disabling the kick FET
         * wait between stopping the FET and reenabling charging in the next
         * state
         */

        HAL_SetLED(MCU_RED, false);

        if (current_kick_type_is_kick) {
            HAL_ClearPin(KICK_PIN);
        } else {
            HAL_ClearPin(CHIP_PIN);
        }

        time.stop_flow_phase--;
    } else if (time.charge_phase >= 0) {
        /**
         * POST KICK COOLDOWN
         * enable charging
         * don't allow kicking during the cooldown
         */

        // reenable charging
        charge_allowed = true;

        time.charge_phase--;
    } else {
        /**
         * IDLE/NOT RUNNING
         * stop timer
         */

        // stop prescaled timer
        TCCR0 &= ~_BV(CS00);
    }
}

void init() {
    // disable interrupts
    cli();

    // disable watchdog
    wdt_reset();
    WDTCR |= (_BV(WDTOE) | _BV(WDE));
    WDTCR &= !_BV(WDE);
    
    // Disable pullups globally
    SFIOR |= _BV(PUD);


    /**
     * LED Initialization
     */

    // Setup default values for output LED pins
    HAL_SetLED(MCU_GREEN, false);
    HAL_SetLED(MCU_YELLOW, true);
    HAL_SetLED(MCU_RED, true);

    // config output pins for status LEDs
    DDRD |= _BV(MCU_GREEN.pin);
    DDRD |= _BV(MCU_YELLOW.pin);
    DDRD |= _BV(MCU_RED.pin);

    // and set ERR/WARN until init done
    // Essentially same as default, but this
    // makes it obvious to people reading this
    HAL_SetLED(MCU_YELLOW, true);
    HAL_SetLED(MCU_RED, true);


    /**
     * Input button initialization
     */

    DDRC &= ~(_BV(DB_SWITCH.pin));
    DDRC &= ~(_BV(DB_CHG_PIN.pin));
    DDRC &= ~(_BV(DB_KICK_PIN.pin));
    DDRC &= ~(_BV(DB_CHIP_PIN.pin));


    /**
     * SPI initialization
     */

    // MISO as output
    // CS and MOSI as input
    DDRB |= _BV(KICK_MISO_PIN.pin);
    DDRB &= ~(_BV(N_KICK_CS_PIN.pin));
    DDRB &= ~(_BV(KICK_MOSI_PIN.pin));


    /**
     * HV Monitor initialization
     */
    // Default values for HV LED display
    HAL_SetLED(HV_IND_MIN, true);
    HAL_SetLED(HV_IND_LOW, true);
    HAL_SetLED(HV_IND_MID, true);
    HAL_SetLED(HV_IND_HIGH, true);
    HAL_SetLED(HV_IND_MAX, true);

    // Voltage monitor pin as input
    // HV LED display as output
    DDRA &= ~(_BV(V_MONITOR_PIN.pin));
    DDRA |= _BV(HV_IND_MIN.pin);
    DDRA |= _BV(HV_IND_LOW.pin);
    DDRA |= _BV(HV_IND_MID.pin);
    DDRA |= _BV(HV_IND_HIGH.pin);
    DDRA |= _BV(HV_IND_MAX.pin);


    /**
     * LT3751 initialization
     */
    // Default values for charge
    HAL_SetPin(LT_CHARGE);
    
    // Charge command output
    // Done and fault as input
    DDRD |= _BV(LT_CHARGE.pin);
    DDRD &= ~(_BV(LT_DONE_N.pin) | _BV(LT_FAULT_N.pin));


    /**
     * Ball sense initialization
     */
    // Default ball sense on startup
    HAL_SetLED(BALL_SENSE_LED, false);
    HAL_ClearPin(BALL_SENSE_TX);

    // tx and led as output
    // RX as intput
    DDRD |= (_BV(BALL_SENSE_TX.pin) | _BV(BALL_SENSE_LED.pin));
    DDRA &= ~(_BV(BALL_SENSE_RX.pin));

    // Enable the LED and TX until first loop
    // This is because you cannot go from {input, tristate} -> {output, high}
    // in a single step
    HAL_SetLED(BALL_SENSE_LED, true);
    HAL_SetPin(BALL_SENSE_TX);
    
    /**
     * JTAG disable
     */
    // Must write twice in 4 cycles to write value to register
    MCUCSR |= _BV(JTD);
    MCUCSR |= _BV(JTD);
    
    /**
     * Enable SPI slave
     */
    // Assume default spi mode
    SPCR = _BV(SPE) | _BV(SPIE);
    SPCR &= ~(_BV(MSTR));  // ensure we are a slave SPI device

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
    TCCR0 |= _BV(WGM01);
    TCCR0 &= ~(_BV(WGM00)); // COM01 - Clear Timer on Compare Match
    OCR0 = TIMING_CONSTANT;  // OCR0A is max val of timer before reset
    ///////////////////////////////////////////////////////////////////////////


    /**
     * ADC Initialization
     */
    // Allow us to just read a single register to get the analog output
    // instead of having to read 2 registers
    // We don't care about the full 10 bit width, only the top 8
    // Setup voltage monitor as input to adc
    ADMUX |= _BV(ADLAR);
    ADMUX |= _BV(V_MONITOR_PIN.pin);

    // Enable adc
    ADCSRA |= _BV(ADEN);

    /**
     * Button logic
     */
    // latch debug state
    in_debug_mode = HAL_IsSet(DB_SWITCH);

    // Turn off the two led's since we finished startup
    HAL_SetLED(MCU_YELLOW, false);
    HAL_SetLED(MCU_RED, false);

    // enable global interrupts
    sei();
}