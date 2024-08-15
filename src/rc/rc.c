/**
 * @brief Remote Control functionality.
 * @ingroup rc
 *
 * This provides command processing from a IR/Radio Remote. It supports 255 codes received from
 * the remote device. It calls a handler function for the received code.
 *
 * Copyright 2024 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "rc.h"
#include "board.h"
#include "debug_support.h"
#include "config/config.h"
#include "cmt/cmt.h"
#include "ui/ui_term.h"

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "nec-rx.pio.h"    // This is the PIO program header for RC6 (Philips) protocol

#include <stdlib.h>
#include <string.h>

#define IR_DATA_MASK        0xFFFF0000  // Mask for the Data portion of the IR Frame
#define IR_DATA_SHIFT       16          // Number of bits to shift to get Data (after mask)
#define IR_DATA_XOR_ADJ     0xFF00      // Value to XOR with Data to get value (after shift)
#define IR_ADDR_MASK        0x0000FFFF  // Mask for the Address portion of the IR Frame
#define IR_ADDR_SHIFT       0           // Number of bits to shift to get Address (after mask)
#define IR_ADDR_XOR_ADJ     0xFF00      // Value to XOR with Address to get value (after shift)
#define IR_FRAME_SAME_MS_DELTA  10      // Frame from 'other' source less than this time delta
#define IR_REPEAT_MS_MIN    50          // Minimum time after last for a valid repeat
#define IR_REPEAT_MS_MAX    150         // Maximum time after last for a valid repeat
#define IR_ADDR_VALUE       0x04        // The IR code address that we expect

typedef struct _rc_lookup_entry_ {
    uint8_t raw_val;
    rc_vcode_t vcode;
} rc_lookup_entry_t;

static rc_lookup_entry_t _nec_rc_lookup[] = {
    {0x08, RC_POWER},
    {0x0B, RC_INPUT},
    {0x1C, RC_MOVE_BACK},
    {0x0F, RC_MENU},
    {0x1B, RC_MENU_3BAR},
    {0x1A, RC_EXIT},
    {0x59, RC_HOME},
    {0x0A, RC_MUTE},
    {0x44, RC_ENTER},
    {0x49, RC_MINUS},
    {0x02, RC_VOL_UP},
    {0x03, RC_VOL_DOWN},
    {0x00, RC_CH_UP},
    {0x01, RC_CH_DOWN},
    {0x10, RC_NUM_0},
    {0x11, RC_NUM_1},
    {0x12, RC_NUM_2},
    {0x13, RC_NUM_3},
    {0x14, RC_NUM_4},
    {0x15, RC_NUM_5},
    {0x16, RC_NUM_6},
    {0x17, RC_NUM_7},
    {0x18, RC_NUM_8},
    {0x19, RC_NUM_9},
    {0x40, RC_CURSOR_UP},
    {0x41, RC_CURSOR_DOWN},
    {0x07, RC_CURSOR_LEFT},
    {0x06, RC_CURSOR_RIGHT},
    {0xFF, RC_OK},
};

// /////////////// Data ////////////////
static remote_code_handler_fn _handlers[CTRL_CODES_NUM];
static rc_ir_frame_t _ir_frame_a_last;
static rc_ir_frame_t _ir_frame_b_last;
static PIO _pio_ir;             // The PIO to use for the IR receivers
static int8_t _pio_irq;         // The interrupt to use
static uint _pio_pgrm_offset;   // The address the PIO program is loaded at

// /// Internal Function Definitions ///
static void _code_zero_handler(uint8_t code, bool repeat);
static void _code_unused_handler(uint8_t code, bool repeat);
static void _handle_ir_frame(cmt_msg_t* msg);
static void _handle_rc_action(cmt_msg_t* msg);
static void _ir_frame_clear(rc_ir_frame_t *frame, rc_ir_source_t src);
static void _ir_frame_copy(rc_ir_frame_t *dest, rc_ir_frame_t *src);
static rc_vcode_t _rc_vcode_from_nec(uint8_t raw);
//
static rc_action_data_t _rc_action;
static bool _rc_action_longpress;
static bool _rc_collecting_value;
static rc_value_entry_t _rc_entry;

const msg_handler_entry_t _os_ir_frame_handler_entry = { MSG_IR_FRAME_RCVD, _handle_ir_frame };
const msg_handler_entry_t _os_rc_action_handler_entry = { MSG_RC_ACTION, _handle_rc_action };

// ////////// IRQ Functions ////////////

static void _on_ir_irq() {
    // IRQ called when the pio fifo for IR-A or IR-B is not empty, i.e. there is data ready
    cmt_msg_t msg_a = { MSG_IR_FRAME_RCVD };
    cmt_msg_t msg_b = { MSG_IR_FRAME_RCVD };
    bool data_was_read;
    do {
        data_was_read = false;
        uint32_t now = now_ms();
        if (!pio_sm_is_rx_fifo_empty(PIO_IR_BLOCK, PIO_IR_A_SM)) {
            uint32_t raw = pio_sm_get(PIO_IR_BLOCK, PIO_IR_A_SM);
            bool repeat = (raw == IR_REPEAT_INDICATOR_FLAG);
            uint16_t data = (repeat ? 0 : ((raw & IR_DATA_MASK) >> IR_DATA_SHIFT) ^ IR_DATA_XOR_ADJ);
            uint16_t addr = (repeat ? 0 : ((raw & IR_ADDR_MASK) >> IR_ADDR_SHIFT) ^ IR_ADDR_XOR_ADJ);
            msg_a.data.ir_frame.data = data;
            msg_a.data.ir_frame.addr = addr;
            msg_a.data.ir_frame.src = IR_A;
            msg_a.data.ir_frame.repeat = repeat;
            msg_a.data.ir_frame.ts_ms = now;
            postBEMsgNoWait(&msg_a);
            data_was_read = true;
        }
        if (!pio_sm_is_rx_fifo_empty(PIO_IR_BLOCK, PIO_IR_B_SM)) {
            uint32_t raw = pio_sm_get(PIO_IR_BLOCK, PIO_IR_B_SM);
            bool repeat = (raw == IR_REPEAT_INDICATOR_FLAG);
            uint16_t data = (repeat ? 0 : ((raw & IR_DATA_MASK) >> IR_DATA_SHIFT) ^ IR_DATA_XOR_ADJ);
            uint16_t addr = (repeat ? 0 : ((raw & IR_ADDR_MASK) >> IR_ADDR_SHIFT) ^ IR_ADDR_XOR_ADJ);
            msg_b.data.ir_frame.data = data;
            msg_b.data.ir_frame.addr = addr;
            msg_b.data.ir_frame.src = IR_B;
            msg_b.data.ir_frame.repeat = repeat;
            msg_b.data.ir_frame.ts_ms = now;
            postBEMsgNoWait(&msg_b);
            data_was_read = true;
        }
    } while(data_was_read);
}


// //////// Internal Functions /////////

static void _code_unused_handler(uint8_t code, bool repeat) {
    ui_term_printf("Remote: %0.3d Repeat: %0.1d (unused)\n", code, repeat);
}

static void _code_zero_handler(uint8_t code, bool repeat) {
    ui_term_printf("Remote: 000\n");
}

static void _handle_code(int16_t code) {
    bool repeat = (code < 0); // Negative values indicate a repeated code
    uint8_t c = abs(code) % CTRL_CODE_MAX;
    remote_code_handler_fn handler = _handlers[c];
    if (0 == c) {
        _code_zero_handler(c, repeat);
    }
    else if (handler) {
        handler(c, repeat);
    }
    else {
        _code_unused_handler(c, repeat);
    }
}

static void _handle_ir_frame(cmt_msg_t* msg) {
    rc_ir_frame_t frame = msg->data.ir_frame;
    uint32_t ts = frame.ts_ms;
    uint16_t data = frame.data;
    uint16_t addr = frame.addr;
    rc_ir_source_t src = frame.src;
    bool repeat = frame.repeat;
    if (debug_mode_enabled()) {
        char* ir_src = (src == IR_A ? "A" : "B");
        char* r_str = (repeat ? " Repeat Last" : "");
        debug_printf(false, "IR-%s: ADDR=%04X DATA=%04X TS=%d%s\n", ir_src, addr, data, ts, r_str);
    }
    // See if it is valid
    rc_ir_frame_t *last = (src == IR_A ? &_ir_frame_a_last : &_ir_frame_b_last);
    if (repeat) {
        // A valid repeat is between 50ms to 150ms after the previous
        int32_t delta_t = frame.ts_ms - last->ts_ms;
        if (delta_t < IR_REPEAT_MS_MIN || delta_t > IR_REPEAT_MS_MAX) {
            goto Ir_Frame_Err_Exit;
        }
        // Store that this was a repeat and update the timestamp
        last->repeat = true;
        last->ts_ms = frame.ts_ms;
    }
    else {
        // Valid data frames have the upper and lower bytes the same
        if (((addr & 0x00FF) != ((addr & 0xFF00) >> 8)) || ((data & 0x00FF) != ((data & 0xFF00) >> 8))) {
            goto Ir_Frame_Err_Exit;
        }
        // The values are okay, see if the address is what we expect
        if ((addr & 0x00FF) != IR_ADDR_VALUE) {
            goto Ir_Frame_Err_Exit;
        }
        _ir_frame_copy(last, &frame);
    }
    // If we make it here, all is good. Finally, check to see if this is an A or B that is the same
    //  ('same' is considered to be the same addr and data from the 'other' IR receiver)
    rc_ir_frame_t *other = (last == &_ir_frame_a_last ? &_ir_frame_b_last : &_ir_frame_a_last);
    if (last->addr == other->addr && last->data == other->data && last->repeat == other->repeat && (last->ts_ms - other->ts_ms < IR_FRAME_SAME_MS_DELTA)) {
        // This frame (the 'last' value) is a repeat of the other one, so don't process it.
        return;
    }
    rc_vcode_t vcode = _rc_vcode_from_nec(last->data & 0x00FF);
    if (vcode == RC_NULL) {
        goto Ir_Frame_Err_Exit;
    }
    cmt_msg_t m = { MSG_RC_ACTION, {0} };
    m.data.rc_action.code = vcode;
    m.data.rc_action.repeat = repeat;
    m.data.rc_action.ts_ms = ts;
    postBothMsgNoWait(&m);
    return;

Ir_Frame_Err_Exit:
    _ir_frame_clear(last, src);
    return;
}

static void _copy_rc_action(rc_action_data_t *dest, rc_action_data_t *src) {
    dest->code = src->code;
    dest->repeat = src->repeat;
    dest->ts_ms = src->ts_ms;
}

/**
 * @brief Handle RC actions (processed from the Remote Control)
 * @ingroup be
 *
 * Keep track of the last code and repeat to detect a 'long press'.
 * Post a message on long press and repeat.
 *
 * Accumulate number entry, clearing if idle time expires. Post a
 * message if there is an accumulated number and a non-number is pressed.
 *
 * @param msg Contains a rc_action_data_t with the action details
 */
static void _handle_rc_action(cmt_msg_t* msg) {
    bool repeat = msg->data.rc_action.repeat;
    uint32_t ts = msg->data.rc_action.ts_ms;
    rc_vcode_t code = msg->data.rc_action.code;
    if (!repeat || code != _rc_action.code) {
        // Save the new code, reset the repeat count, indicate no repeat (regardless)
        //  we use the 'repeat' for a higher level indicator after 'long press'
        _rc_action.code = code;
        _rc_action.repeat = false;
        _rc_action.ts_ms = ts;
        _rc_action_longpress = false;
        // Now, check for numeric entry
        switch (code) {
            case RC_NUM_0:
            case RC_NUM_1:
            case RC_NUM_2:
            case RC_NUM_3:
            case RC_NUM_4:
            case RC_NUM_5:
            case RC_NUM_6:
            case RC_NUM_7:
            case RC_NUM_8:
            case RC_NUM_9:
                // Adjust the accumulator and add this in
                _rc_entry.value = (_rc_entry.value * 10) + (code - RC_NUM_0);
                _rc_collecting_value = true;
                break;
            case RC_MINUS:
                // If we aren't collecting digits, start and set divisor to -1
                // else, treat the '-' button as a 'point'.
                if (!_rc_collecting_value) {
                    _rc_collecting_value = true;
                    _rc_entry.divisor = -1;
                }
                else if (abs(_rc_entry.divisor) == 1) {
                    _rc_entry.divisor *= 10;
                }
                break;
            default:
                // The button wasn't a number or '-', post the number if collecting
                if (_rc_collecting_value) {
                    cmt_msg_t msg = { MSG_RC_VALUE_ENTERED };
                    msg.data.rc_entry.code = code;
                    msg.data.rc_entry.value = _rc_entry.value;
                    msg.data.rc_entry.divisor = _rc_entry.divisor;
                    postBothMsgNoWait(&msg);
                    // An app responding to the entered value must call rc_value_collecting_reset
                    // to collect another value.
                }
                break;
        };
    }
    else {
        if (repeat) {
            uint32_t deltat = ts - _rc_action.ts_ms;
            bool post_msg = false;
            if (!_rc_action_longpress) {
                // Not yet a 'long press'
                if (deltat >= config_current()->long_press) {
                    // It is now a 'long press'. Post a message.
                    _rc_action_longpress = true;
                    _rc_action.ts_ms = ts;
                    post_msg = true;
                }
            }
            else {
                // it was already a 'long press', see if it's long enough to consider it a repeat
                if (deltat >= SWITCH_REPEAT_MS) {
                    _rc_action.repeat = true;
                    _rc_action.ts_ms = ts;
                    post_msg = true;
                }
            }
            if (post_msg) {
                cmt_msg_t msg = { MSG_RC_LONGPRESS };
                _copy_rc_action(&msg.data.rc_action, &_rc_action);
                postBothMsgNoWait(&msg);
            }
        }
    }
}


static void _ir_frame_clear(rc_ir_frame_t *frame, rc_ir_source_t src) {
    frame->addr = 0;
    frame->data = 0;
    frame->src = src;
    frame->repeat = false;
    frame->ts_ms = 0;
}

static void _ir_frame_copy(rc_ir_frame_t *dest, rc_ir_frame_t *src) {
    dest->addr = src->addr;
    dest->data = src->data;
    dest->src = src->src;
    dest->repeat = src->repeat;
    dest->ts_ms = src->ts_ms;
}

static rc_vcode_t _rc_vcode_from_nec(uint8_t raw) {
    rc_vcode_t vcode = RC_NULL;
    for (int i = 0; i < sizeof(_nec_rc_lookup); i++) {
        if (raw == _nec_rc_lookup[i].raw_val) {
            vcode = _nec_rc_lookup[i].vcode;
            break;
        }
    }
    return vcode;
}

// Public functions

void rc_enable_ir(bool ir_a_enabled, bool ir_b_enabled) {
    if (ir_a_enabled) {
        pio_sm_set_enabled(_pio_ir, PIO_IR_A_SM, true);
    }
    if (ir_b_enabled) {
        pio_sm_set_enabled(_pio_ir, PIO_IR_B_SM, true);
    }
    if (ir_a_enabled || ir_b_enabled) {
        irq_set_enabled(_pio_irq, true); // Enable the IRQ
    }
}

void rc_handle_code(int16_t code) {
    _handle_code(code);
}

bool rc_is_collecting_value() {
    return _rc_collecting_value;
}

void rc_value_collecting_reset() {
    _rc_collecting_value = false;
    _rc_entry.divisor = 1;
    _rc_entry.value = 0;
}

// //////////// Module Init /////////////
void rc_module_init(bool ir_a_enabled, bool ir_b_enabled) {
    _pio_ir = PIO_IR_BLOCK;
    _rc_action.code = 0;
    _rc_action.repeat = false;
    _rc_action_longpress = false;
    rc_value_collecting_reset();

    memset(_handlers, 0, CTRL_CODES_NUM * sizeof(remote_code_handler_fn));
    _ir_frame_clear(&_ir_frame_a_last, IR_A);
    _ir_frame_clear(&_ir_frame_b_last, IR_B);
    if (ir_a_enabled || ir_b_enabled) {
        // One of the IRs is enabled... Set up the PIO to read the IR ports
        // Set up interrupt
        _pio_irq = PIO_IR_IRQ;
        if (irq_get_exclusive_handler(_pio_irq)) {
            _pio_irq++;
            if (irq_get_exclusive_handler(_pio_irq)) {
                panic("All IRQs are in use");
            }
        }
        const uint irq_index = _pio_irq - PIO1_IRQ_0; // Get index of the IRQ
        irq_add_shared_handler(_pio_irq, _on_ir_irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
        irq_set_enabled(_pio_irq, false); // Disable the IRQ
        _pio_pgrm_offset = pio_add_program(_pio_ir, &nec_rx_program);
        if (ir_a_enabled) {
            nec_rx_program_init(_pio_ir, PIO_IR_A_SM, _pio_pgrm_offset, IR_A_GPIO);
            pio_sm_clear_fifos(_pio_ir, PIO_IR_A_SM);
            // Set pio to tell us when the RX FIFO is NOT empty
            pio_set_irqn_source_enabled(_pio_ir, irq_index, pis_sm0_rx_fifo_not_empty, true);
        }
        if (ir_b_enabled) {
            nec_rx_program_init(_pio_ir, PIO_IR_B_SM, _pio_pgrm_offset, IR_B_GPIO);
            pio_sm_clear_fifos(_pio_ir, PIO_IR_B_SM);
            // Set pio to tell us when the FIFO is NOT empty
            pio_set_irqn_source_enabled(_pio_ir, irq_index, pis_sm1_rx_fifo_not_empty, true);
        }
    }
}