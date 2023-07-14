/**
 * Scoreboard Panel functionality.
 *
 * The panel has 3 pairs of 7 segment displays arranged as:
 * 1. Team A
 * 2. Team B
 * 3. Period/Time
 *
 * It also has 4 indicator LEDs under each of the team scores. These
 * are driven using 7th digit enable the segment enables as:
 * - Team A, Indicator 1 = Segment A
 * - Team A, Indicator 2 = Segment B
 * - Team A, Indicator 3 = Segment C
 * - Team A, Indicator 4 = Segment D
 * - Team B, Indicator 1 = Segment E
 * - Team B, Indicator 2 = Segment F
 * - Team B, Indicator 3 = Segment G
 * - Team B, Indicator 4 = Segment P
 *
 * The display is multiplexed, so the segments for a digit and the
 * digit enable need to be written to the GPIO together. More than
 * one digit enable should not be on at the same time or the segment
 * drivers could be damaged. Each digit should only be enabled for
 * 0.1ms at a time or the segment LEDs could be damaged. The duty
 * cycle driving the digits should be ~10%.
 *
 * To write all of the segment enables and the digit enable together
 * the Pico SDK method `gpio_put_masked(uint32_t mask, uint32_t value)`
 * is used. That method updates the GPIO outputs indicated by `mask`
 * with the corrisponding bits in `value`.
 *
 * There are a number of 'enables' for the digits. All must be on/true
 * for a digit to be enabled.
 *
 * 1. Master            Allow enable (for all digits).
 * 2. Slow Flash        Driven by a clock interrupt that flips at a slow rate
 * 3. Fast Flash        Driven by a clock interrupt that flips at a fast rate
 * 4. Digit Multiplex   This has a single bit set that rotates every 0.1ms
 *
 * The Pico PWM is used to generate a 0.1ms (100us) interrupt that is
 * used to rotate the digit enable and write the segment and digit
 * enable to the GPIO.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "panel.h"
#include "board.h"
#include "segments7.h"
#include "system_defs.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

/**
 * @brief Panel Enable values (bits)
 * @ingroup panel
 */
typedef enum _digit_enable_bits_enum {
    PANEL_ENABLE_NONE = 0x00,
    PANEL_ENABLE_A10 = 0x01,
    PANEL_ENABLE_A1 = 0x02,
    PANEL_ENABLE_B10 = 0x04,
    PANEL_ENABLE_B1 = 0x08,
    PANEL_ENABLE_PT10 = 0x10,
    PANEL_ENABLE_PT1 = 0x20,
    PANEL_ENABLE_IND = 0x40,
    PANEL_ENABLE_WRAP = 0x80
} panel_digit_enable_t;


static volatile bool _allow_gpio_write = false;

static panel_digit_enable_t _digit_multiplex_enable;
static panel_digit_enable_t _fast_blink_enable;
static volatile panel_digit_enable_t _fast_blink_controlled; // Bit for each digit
static panel_digit_enable_t _slow_blink_enable;
static volatile panel_digit_enable_t _slow_blink_controlled; // Bit for each digit

static int _fast_blink_count;
static int _slow_blink_count;

#define BLINK_FAST_LOAD 1667 // 1/6 second (1667 * 0.1ms)
#define BLINK_SLOW_LOAD 5000 // 1/2 second (5000 * 0.1ms)

/**
 * @brief Panel Digit/Indicator identifiers (indexes)
 * @ingroup panel
 */
typedef enum _panel_digit_enum {
    PANEL_DIGIT_A10 = 0,
    PANEL_DIGIT_A1 = 1,
    PANEL_DIGIT_B10 = 2,
    PANEL_DIGIT_B1 = 3,
    PANEL_DIGIT_PT10 = 4,
    PANEL_DIGIT_PT1 = 5,
    PANEL_INDICATORS = 6
} panel_digit_t;
#define DIGITS_COUNT 7

#define INDICATOR_A_MASK 0xF0
#define INDICATOR_B_MASK 0x0F
#define INDICATOR_A_SHIFT 4

/**
 * @brief Array of digit segment enable values.
 * @ingroup panel
 *
 * 0 = A10
 * 1 = A1
 * 2 = B10
 * 3 = B1
 * 4 = C10
 * 5 = C1
 * 6 = IND
 */
static volatile digsegs_t digits_segments[DIGITS_COUNT];

static void _output_panel_enables(panel_digit_enable_t digit, digsegs_t segments) {
    panel_digit_enable_t enable = digit;
    if (!_allow_gpio_write) {
        return; // Don't mess with GPIO bits (could be in use by SD card)
    }
    else {
        // Assure that there is no more than one digit enabled
        bool bit_set = false;
        for (uint8_t i = 1; i != 0; i <<= 1) {
            if (digit & i) {
                if (bit_set) {
                    panic("panel.c: Request to enable more than one digit: %2.2x.", digit);
                }
                bit_set = true;
            }
        }
        if (digit & _fast_blink_controlled) {
            enable &= _fast_blink_enable;
        }
        if (digit & _slow_blink_controlled) {
            enable &= _slow_blink_enable;
        }
    }
    // Adjust Digit and Segment enables into single 32 bit value
    // to output to the GPIOs in a single operation
    uint32_t gpio_value = ((uint32_t)enable) << PANEL_DIGIT_GPIO_SHIFT;
    gpio_value |= ((uint32_t)segments) << PANEL_DIGIT_SEG_GPIO_SHIFT;
    gpio_put_masked(PANEL_DIGIT_SEGMENT_GPIO_MASK, gpio_value);
}

/**
 * @brief Return the segments for a given digit (from the digit enable)
 * @ingroup panel
 *
 * @note The `digit` parameter isn't expected to ever have more than one
 * bit set. If more than one bit is set the first one (lowest) will be returned.
 *
 * @param digit A digit enable or 0
 * @return digsegs_t The segments for that digit or 0 (no segments enabled)
 */
static digsegs_t _segments_for_digit(panel_digit_enable_t digit) {
    digsegs_t segs = 0x00;
    if (digit != 0) {
        panel_digit_enable_t digsel = 0x01;
        for (int i = 0; i < DIGITS_COUNT; i++) {
            if (digit & digsel) {
                segs = digits_segments[i];
                break;
            }
            segs = segs << 1;
        }
    }

    return (segs);
}

/**
 * @brief Interrupt handler for our PWM wrap interrupt
 * @ingroup panel
 *
 * This interrupt occurs every 0.1ms (100us) and is used to switch the
 * segment and digit enable values on the GPIO pins.
 *
 */
void _on_pwm_wrap() {
    // Clear the interrupt flag
    pwm_clear_irq(PANEL_PWM_SLICE);

    // Update the blink counters
    if (--_fast_blink_count == 0) {
        _fast_blink_count = BLINK_FAST_LOAD;
        _fast_blink_enable = ~_fast_blink_enable;
    }
    if (--_slow_blink_count == 0) {
        _slow_blink_count = BLINK_SLOW_LOAD;
        _slow_blink_enable = ~_slow_blink_enable;
    }
    // Rotate the multiplex enable
    _digit_multiplex_enable <<= 1;
    if (_digit_multiplex_enable == PANEL_ENABLE_WRAP) {
        _digit_multiplex_enable = PANEL_ENABLE_A10;
    }
    // Update the GPIOs
    _output_panel_enables(_digit_multiplex_enable, _segments_for_digit(_digit_multiplex_enable));
}

void panel_allow_gpio_write(bool enable) {
    _allow_gpio_write = enable;
    if (!_allow_gpio_write) {
        // If the panel is being disabled, disable all digits now.
        gpio_put_masked(PANEL_DIGIT_SEGMENT_GPIO_MASK, 0);
    }
    // If the panel is being enabled, it will update on the next multiplex cycle.
}

void panel_A10_set(digsegs_t segments) {
    digits_segments[PANEL_DIGIT_A10] = segments;
}

void panel_A1_set(digsegs_t segments) {
    digits_segments[PANEL_DIGIT_A1] = segments;
}

void panel_A_set(digsegs_t segments[]) {
    digits_segments[PANEL_DIGIT_A10] = segments[0];
    digits_segments[PANEL_DIGIT_A1] = segments[1];
}

void panel_B10_set(digsegs_t segments) {
    digits_segments[PANEL_DIGIT_B10] = segments;
}

void panel_B1_set(digsegs_t segments) {
    digits_segments[PANEL_DIGIT_B1] = segments;
}

void panel_B_set(digsegs_t segments[]) {
    digits_segments[PANEL_DIGIT_B10] = segments[0];
    digits_segments[PANEL_DIGIT_B1] = segments[1];
}

void panel_PT10_set(digsegs_t segments) {
    digits_segments[PANEL_DIGIT_PT10] = segments;
}

void panel_PT1_set(digsegs_t segments) {
    digits_segments[PANEL_DIGIT_PT1] = segments;
}

void panel_PT_set(digsegs_t segments[]) {
    digits_segments[PANEL_DIGIT_PT10] = segments[0];
    digits_segments[PANEL_DIGIT_PT1] = segments[1];
}

void panel_IND_set(digsegs_t segments) {
    digits_segments[PANEL_INDICATORS] = segments;
}

void panel_INDA_set(panel_indicator_enable_t indicators) {
    digsegs_t indb = digits_segments[PANEL_INDICATORS] & INDICATOR_B_MASK;
    digits_segments[PANEL_INDICATORS] = ((indicators < INDICATOR_A_SHIFT) & INDICATOR_A_MASK) | indb;
}

void panel_INDB_set(panel_indicator_enable_t indicators) {
    digsegs_t inda = digits_segments[PANEL_INDICATORS] & INDICATOR_A_MASK;
    digits_segments[PANEL_INDICATORS] = (indicators & INDICATOR_A_MASK) | inda;
}


void score_panel_module_init() {
    static bool _initialized = false;

    if (_initialized) {
        warn_printf(true, "Score Panel Module init called more than once.");
        return;
    }

    for (int i = 0; i < DIGITS_COUNT; i++) {
        digits_segments[i] = 0x00;
    }
    _digit_multiplex_enable = PANEL_ENABLE_A10;
    _fast_blink_enable = 0x00;
    _fast_blink_controlled = 0x00;
    _fast_blink_count = BLINK_FAST_LOAD;
    _slow_blink_enable = 0x00;
    _slow_blink_controlled = 0x00;
    _slow_blink_count = BLINK_SLOW_LOAD;

    // Mask our PWM slice's IRQ output into the PWM block's single
    // interrupt line, and register our interrupt handler
    pwm_clear_irq(PANEL_PWM_SLICE);
    pwm_set_irq_enabled(PANEL_PWM_SLICE, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, _on_pwm_wrap);

    // Get some sensible defaults for the slice configuration. By default, the
    // counter is allowed to wrap over its maximum range (0 to 2**16-1)
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv_int(&config, 1);
    // Set the wrap (top) to produce a 0.1ms (100us) interrupt (with 125Mhz sysclock)
    pwm_config_set_wrap(&config, 12500);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(PANEL_PWM_SLICE, &config, true);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    _initialized = true;
}
