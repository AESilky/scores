/**
 * Scoreboard Panel functionality.
 *
 * There are two types of panel, NUMERIC and LINEAR.
 * 
 * The NUMERIC panel has 3 pairs of 7 segment displays arranged as:
 * 1. Team A
 * 2. Team B
 * 3. Period/Time (C digits)
 *
 * It also has 4 indicator LEDs under each of the team scores. These
 * are driven using a 7th digit enable with the segment enables as:
 * - Team A, Indicator 1 = Segment A
 * - Team A, Indicator 2 = Segment B
 * - Team A, Indicator 3 = Segment C
 * - Team A, Indicator 4 = Segment D
 * - Team B, Indicator 1 = Segment E
 * - Team B, Indicator 2 = Segment F
 * - Team B, Indicator 3 = Segment G
 * - Team B, Indicator 4 = Segment P
 *
 * The LINEAR panel has two columns of 24 'dots', one for team A and
 * one for team B. There is an additional row of 8 dots that can be
 * used to show the number of games won, or whatever.
 * 
 * The columns use:
 * - Team A  1-8  = A01 Segment P - A
 * - Team A  9-16 = A10 Segment P - A
 * - Team A 15-24 = C10 Segment P - A
 * - Team B  1-8  = B01 Segment P - A
 * - Team B  9-16 = B10 Segment P - A
 * - Team B 15-24 = C01 Segment P - A
 * The row uses:
 * - Left to Right = Indicator Segment A - P
 * 
 * The display is multiplexed, so the segments for a digit and the
 * digit enable need to be written to the GPIO together. More than
 * one digit enable should not be on at the same time or the segment
 * drivers could be damaged. Each digit should only be enabled for
 * 0.1ms at a time or the segment LEDs could be damaged. The duty
 * cycle driving the digits should be ~10%.
 *
 * There are a number of 'enables' for the digits. All must be on/true
 * for a digit to be enabled.
 *
 * 1. Master            Allow enable (for all digits).
 * 2. Digit             Control for each digit individually
 * 2. Slow Flash        Driven by the DMA scan-end interrupt that flips at a slow rate
 * 3. Fast Flash        Driven by the DMA scan-end interrupt that flips at a fast rate
 *
 * This module takes care of the physical display panel. The logic/decision
 * of what to put on the display is not in this module.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "panel.h"
#include "panel_msg_hndlr.h"

#include "board.h"
#include "system_defs.h"

#include "panel/segments7/segments7.h"

#include "hardware/dma.h" //The hardware DMA library
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

/////////////////////////////////////////////////////////////////////
// Message Handling
/////////////////////////////////////////////////////////////////////
//
void _panel_slowblnk_handler(cmt_msg_t* msg) {
    bool on = msg->data.bv;
    led_on(on);
}
const msg_handler_entry_t _panel_slowblnk_handler_entry = { MSG_PANEL_BLINK_SLOW_TGL, _panel_slowblnk_handler };

/////////////////////////////////////////////////////////////////////
// Panel Control
/////////////////////////////////////////////////////////////////////
//
#define LINEAR_01TO08_MASK 0x0000FF
#define LINEAR_01TO08_SHIFT 0
#define LINEAR_09TO16_MASK 0x00FF00
#define LINEAR_09TO16_SHIFT 8
#define LINEAR_17TO24_MASK 0xFF0000
#define LINEAR_17TO24_SHIFT 16

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
    PANEL_ENABLE_C10 = 0x10,
    PANEL_ENABLE_C1 = 0x20,
    PANEL_ENABLE_IND = 0x40,
    PANEL_ENABLE_FILL = 0x80    // Not used. Just to fill out the bits
} panel_digit_enable_t;

static panel_type_t _panel_type;

static bool _fast_blink_enable;
static volatile panel_digit_enable_t _fast_blink_digit_ctrl; // Bit for each digit
static bool _slow_blink_enable;
static volatile panel_digit_enable_t _slow_blink_digit_ctrl; // Bit for each digit

#define BLINK_FAST_LOAD 238 // 1/5 second (238 * 0.840ms)
#define BLINK_SLOW_LOAD 595 // 1/2 second (595 * 0.840ms)
#define REPETITIVE_LOAD  25 // For 21ms repetitive message

static int _repetitive_count;
static int _fast_blink_count;
static int _slow_blink_count;

#define DIGITS_COUNT 8

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
volatile digsegs_t _digits_segments[DIGITS_COUNT];
volatile bool _segments_changed;

#define DIGITS_CTRL_BUF_SIZE DIGITS_COUNT //The size of the value+enable bytes for the 6 digits, indicators, and a blank

volatile uint16_t _digits_ctrl_buf[DIGITS_CTRL_BUF_SIZE] __attribute__ ((aligned(16)));
#define DCB_SEGS_MASK 0x00FF
#define DCB_DE_MASK 0xFF00
int _dma_channel_panel;     //The DMA channel to drive the panel
int _dma_channel_control;   //The DMA channel that reloads the panel channel
uint32_t _tran_count;       //The DMA transaction count

PIO _pio_panel;             // The PIO to use for the panel

/**
 * @brief Interrupt handler for our control DMA channel interrupt
 * @ingroup panel
 *
 * This interrupt occurs every 0.84ms (105us * 8) and is used to
 * post a recurring message every 21ms (scan-end * 25) and to blink
 * the display digits.
 *
 */
void _on_dma_irq() {
    // Interrupt at end of 'scan' when the 2nd channel re-programs the 1st.
    bool post_blink_fast = false;
    bool post_blink_slow = false;
    bool post_repetitive = false;

    // Clear the interrupt request.
    dma_hw->ints1 = 1u << _dma_channel_control;

    // Update the repetitive message counter
    if (--_repetitive_count == 0) {
        _repetitive_count = REPETITIVE_LOAD;
        post_repetitive = true;
    }
    // Update the blink counters
    if (--_fast_blink_count == 0) {
        _fast_blink_count = BLINK_FAST_LOAD;
        _fast_blink_enable = !_fast_blink_enable;
        post_blink_fast = true;
    }
    if (--_slow_blink_count == 0) {
        _slow_blink_count = BLINK_SLOW_LOAD;
        _slow_blink_enable = !_slow_blink_enable;
        post_blink_slow = true;
    }

    if (_segments_changed) {
        // Put the segments into the digits control buffer
        _segments_changed = false;
        for (int i=0; i < DIGITS_COUNT; i++) {
            digsegs_t v = _digits_segments[i];
            uint16_t ev = _digits_ctrl_buf[i];
            ev = (ev & DCB_DE_MASK) | v;
            _digits_ctrl_buf[i] = ev;
        }
    }

    // Post the messages.
    if (post_repetitive) {
        cmt_msg_t msg1 = { MSG_PANEL_REPEAT_21MS };
        postBothMsgNoWait(&msg1);
    }
    if (post_blink_fast) {
        cmt_msg_t msg2 = { MSG_PANEL_BLINK_FAST_TGL };
        msg2.data.bv = _fast_blink_enable;
        postBothMsgNoWait(&msg2);
    }
    if (post_blink_slow) {
        cmt_msg_t msg3 = { MSG_PANEL_BLINK_SLOW_TGL };
        msg3.data.bv = _slow_blink_enable;
        postBothMsgNoWait(&msg3);
    }
}


/**
 * @brief Return the segments for a given digit (from the digit enable)
 * @ingroup panel
 *
 * @note The `digit` parameter isn't expected to ever have more than one
 * bit set. If more than one bit is set the first one (lsb) will be returned.
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
                segs = _digits_segments[i];
                break;
            }
            segs = segs << 1;
        }
    }

    return (segs);
}

void panel_blank() {
    _digits_segments[PANEL_DIGIT_A10] = 0x00;
    _digits_segments[PANEL_DIGIT_A1] = 0x00;
    _digits_segments[PANEL_DIGIT_B10] = 0x00;
    _digits_segments[PANEL_DIGIT_B1] = 0x00;
    _digits_segments[PANEL_DIGIT_C10] = 0x00;
    _digits_segments[PANEL_DIGIT_C1] = 0x00;
    _digits_segments[PANEL_INDICATORS] = 0x00;
    _segments_changed = true;
}

void panel_A10_set(digsegs_t segments) {
    _digits_segments[PANEL_DIGIT_A10] = segments;
    _segments_changed = true;
}

void panel_A1_set(digsegs_t segments) {
    _digits_segments[PANEL_DIGIT_A1] = segments;
    _segments_changed = true;
}

void panel_A_set(digsegs_t segments[]) {
    _digits_segments[PANEL_DIGIT_A10] = segments[0];
    _digits_segments[PANEL_DIGIT_A1] = segments[1];
    _segments_changed = true;
}

void panel_B10_set(digsegs_t segments) {
    _digits_segments[PANEL_DIGIT_B10] = segments;
    _segments_changed = true;
}

void panel_B1_set(digsegs_t segments) {
    _digits_segments[PANEL_DIGIT_B1] = segments;
    _segments_changed = true;
}

void panel_B_set(digsegs_t segments[]) {
    _digits_segments[PANEL_DIGIT_B10] = segments[0];
    _digits_segments[PANEL_DIGIT_B1] = segments[1];
    _segments_changed = true;
}

void panel_C10_set(digsegs_t segments) {
    _digits_segments[PANEL_DIGIT_C10] = segments;
    _segments_changed = true;
}

void panel_C1_set(digsegs_t segments) {
    _digits_segments[PANEL_DIGIT_C1] = segments;
    _segments_changed = true;
}

void panel_C_set(digsegs_t segments[]) {
    _digits_segments[PANEL_DIGIT_C10] = segments[0];
    _digits_segments[PANEL_DIGIT_C1] = segments[1];
    _segments_changed = true;
}

void panel_IND_set(digsegs_t segments) {
    _digits_segments[PANEL_INDICATORS] = segments;
    _segments_changed = true;
}

void panel_INDA_set(panel_indicator_enable_t indicators) {
    digsegs_t indb = _digits_segments[PANEL_INDICATORS] & INDICATOR_B_MASK;
    _digits_segments[PANEL_INDICATORS] = ((indicators < INDICATOR_A_SHIFT) & INDICATOR_A_MASK) | indb;
    _segments_changed = true;
}

void panel_INDB_set(panel_indicator_enable_t indicators) {
    digsegs_t inda = _digits_segments[PANEL_INDICATORS] & INDICATOR_A_MASK;
    _digits_segments[PANEL_INDICATORS] = (indicators & INDICATOR_A_MASK) | inda;
    _segments_changed = true;
}

void panel_LinearA_set(linedots_t dots) {
    _digits_segments[PANEL_DIGIT_A1]  = (digsegs_t)((dots & LINEAR_01TO08_MASK) >> LINEAR_01TO08_SHIFT);
    _digits_segments[PANEL_DIGIT_A10] = (digsegs_t)((dots & LINEAR_09TO16_MASK) >> LINEAR_09TO16_SHIFT);
    _digits_segments[PANEL_DIGIT_C10] = (digsegs_t)((dots & LINEAR_17TO24_MASK) >> LINEAR_17TO24_SHIFT);
    _segments_changed = true;
}

void panel_LinearB_set(linedots_t dots) {
    _digits_segments[PANEL_DIGIT_A1]  = (digsegs_t)((dots & LINEAR_01TO08_MASK) >> LINEAR_01TO08_SHIFT);
    _digits_segments[PANEL_DIGIT_A10] = (digsegs_t)((dots & LINEAR_09TO16_MASK) >> LINEAR_09TO16_SHIFT);
    _digits_segments[PANEL_DIGIT_C10] = (digsegs_t)((dots & LINEAR_17TO24_MASK) >> LINEAR_17TO24_SHIFT);
    _segments_changed = true;
}

void panel_digit_blink_fast_add(panel_digit_t digit) {
    _fast_blink_digit_ctrl |= (1u << digit);
}

void panel_digit_blink_fast_remove(panel_digit_t digit) {
    _fast_blink_digit_ctrl &= ~(1u << digit);
}

void panel_digit_blink_slow_add(panel_digit_t digit) {
    _slow_blink_digit_ctrl |= (1u << digit);
}

void panel_digit_blink_slow_remove(panel_digit_t digit) {
    _slow_blink_digit_ctrl &= ~(1u << digit);
}

linedots_t panel_linedots_for_value(uint8_t value) {
    linedots_t dots = 0;
    value = (value <= 24 ? value : 24);
    for (int i=0; i < value; i++) {
        // Shift the current value to the right and set bit 0;
        dots <<= 1;
        dots |= 0x000001;
    }
    return (dots);
}

panel_type_t panel_type() {
    return _panel_type;
}

void panel_module_init(panel_type_t panel_type) {
    static bool _initialized = false;

    if (_initialized) {
        warn_printf(true, "Score Panel Module init called more than once.");
        return;
    }

    _panel_type = panel_type;

    for (int i = 0; i < DIGITS_COUNT; i++) {
        _digits_segments[i] = 0xFF;
    }
    _segments_changed = true;
    _fast_blink_enable = false;
    _fast_blink_digit_ctrl = 0x00;
    _fast_blink_count = BLINK_FAST_LOAD;
    _slow_blink_enable = false;
    _slow_blink_digit_ctrl = 0x00;
    _slow_blink_count = BLINK_SLOW_LOAD;
    _repetitive_count = REPETITIVE_LOAD;

    _pio_panel = PIO_PANEL_DRIVE_BLOCK;
    _tran_count = DIGITS_CTRL_BUF_SIZE;

    // Create the PIO program. This simply reads a word from the fifo and outputs 15 bits to the GPIO
    // This is just a single `out pins, n`
    // instruction with a wrap.
    uint paneldrv_pio_prog = pio_encode_out(pio_pins, PANEL_PIO_GPIO_COUNT);
    struct pio_program paneldrv_prog = {
            .instructions = (const uint16_t *)&paneldrv_pio_prog,
            .length = 1,
            .origin = -1
    };
    uint offset = pio_add_program(_pio_panel, &paneldrv_prog);

    // Configure state machine to loop over this `out` instruction forever,
    // with autopull enabled.
    for (int i=0; i < PANEL_PIO_GPIO_COUNT; i++) {
            pio_gpio_init(_pio_panel, PANEL_PIO_GPIO_BASE + i);
    }
    pio_sm_set_consecutive_pindirs(_pio_panel, PIO_PANEL_DRIVE_SM, PANEL_PIO_GPIO_BASE, PANEL_PIO_GPIO_COUNT, true);
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_out_pins(&c, PANEL_PIO_GPIO_BASE, PANEL_PIO_GPIO_COUNT);
    sm_config_set_wrap(&c, offset, offset);
    sm_config_set_clkdiv(&c, 1.0f);
    sm_config_set_out_shift(&c, true, true, PANEL_PIO_GPIO_COUNT);
    pio_sm_init(_pio_panel, PIO_PANEL_DRIVE_SM, offset, &c);

    // Initialize the digits control buffer with alternating segments on and stepping through the enables
    uint8_t segs = 0;
    for (int i = 0; i < DIGITS_CTRL_BUF_SIZE; i++) {
        // if (i % 2 == 0) {segs = 0x6E;} else {segs = 0x61;}
        if (i % 2 == 0) {segs = 0xFF;} else {segs = 0xFF;}
        uint16_t de = (1 << i);
        uint16_t v = (de << 8) + segs; // Store the digit enable and segments
        _digits_ctrl_buf[i] = v;
    }
    _digits_ctrl_buf[DIGITS_CTRL_BUF_SIZE - 1] = 0x0000; // Zero out the last word for the safe-fill

    _dma_channel_panel = dma_claim_unused_channel(true);
    _dma_channel_control = dma_claim_unused_channel(true);

    dma_channel_config c1 = dma_channel_get_default_config(_dma_channel_control); //Get configurations for the control channel
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32); //Set control channel data transfer size to 32 bits
    channel_config_set_read_increment(&c1, false); //Set control channel read increment to false
    channel_config_set_write_increment(&c1, false); //Set control channel write increment to false
    // Configure control channel to write to panel channel's al1_transfer_count_trig register
    dma_channel_configure(_dma_channel_control, &c1,
        &dma_hw->ch[_dma_channel_panel].al1_transfer_count_trig,    // Trigger the panel DMA
        &_tran_count,                                               // The value to write
        1,                                                          // Transfer count
        false);                                                     // Don't start yet

    dma_channel_config c2 = dma_channel_get_default_config(_dma_channel_panel); //Get configurations for the panel channel
    channel_config_set_transfer_data_size(&c2, DMA_SIZE_16); //Set panel channel data transfer size to 16 bits
    channel_config_set_read_increment(&c2, true); //Set panel channel read increment to true
    channel_config_set_write_increment(&c2, false); //Set panel channel write increment to false

    // Configure the dma timer to transfer at a rate of once every 100 microseconds (10kHz)
    int panel_dreq_timer = dma_claim_unused_timer(true);
    dma_timer_set_fraction(panel_dreq_timer, 5, 65535); // (5/65535)*125000000 = 9537 (105 microseconds)
    uint timer_dreq_id = dma_get_timer_dreq(panel_dreq_timer);

    channel_config_set_dreq(&c2, timer_dreq_id); //Set the transfer request signal.
    channel_config_set_chain_to(&c2, _dma_channel_control); //When the panel channel completes, trigger the control channel
    channel_config_set_ring(&c2, false, 4); //Set read address wrapping to 4 bits (16 bytes)

    // Configure data channel to write to the PIO driving the panel
    dma_channel_configure(_dma_channel_panel, &c2,
        &_pio_panel->txf[PIO_PANEL_DRIVE_SM],                       // Destination
        _digits_ctrl_buf,                                           // Memory buffer to read from
        DIGITS_CTRL_BUF_SIZE,                                       // Number of bytes to transfer in one block
        false);                                                     // Don't start yet

    // Tell the DMA to raise IRQ line 1 when the channel finishes a block (used to time blinking)
    dma_channel_set_irq1_enabled(_dma_channel_control, true);

    // Configure the processor to run _on_dma_irq() when DMA IRQ 1 is asserted
    irq_set_exclusive_handler(DMA_IRQ_1, _on_dma_irq);
    irq_set_enabled(DMA_IRQ_1, true);

    // Start the PIO
    pio_sm_set_enabled(_pio_panel, PIO_PANEL_DRIVE_SM, true);
    // Start the control channel. This will start the panel channel.
    dma_start_channel_mask(1u << _dma_channel_control);

    _initialized = true;
}
