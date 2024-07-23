/**
 * scores Back-End - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "be.h"

#include "board.h"
#include "debug_support.h"

#include "cmt/cmt.h"
#include "config/config.h"
#include "curswitch/curswitch.h"
#include "net/net.h"
#include "panel/panel.h"
#include "term/term.h"
#include "util/util.h"

#include "hardware/rtc.h"

#include "panel/panel_msg_hndlr.h"

#define _BE_STATUS_PULSE_PERIOD 6999

static switch_id_t _bank1_sw_pressed = SW_NONE;
static cmt_msg_t _bank1_sw_longpress_msg = { MSG_B1SW_LONGPRESS_DELAY };
static switch_id_t _bank2_sw_pressed = SW_NONE;
static cmt_msg_t _bank2_sw_longpress_msg = { MSG_B2SW_LONGPRESS_DELAY };
static bool _input_sw_pressed;
static cmt_msg_t _input_sw_debounce_msg = { MSG_INPUT_SW_DEBOUNCE };
static config_t* _last_cfg;
static bool _ui_initialized = false;

// Message handler functions...
static void _handle_be_test(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_cmt_sleep(cmt_msg_t* msg);
static void _handle_input_sw_debounce(cmt_msg_t* msg);
static void _handle_panel_repeat_21ms(cmt_msg_t* msg);
static void _handle_switch_action(cmt_msg_t* msg);
static void _handle_switch_longpress_delay(cmt_msg_t* msg);
static void _handle_ui_initialized(cmt_msg_t* msg);

// Idle functions...
static void _be_idle_function_1();
static void _be_idle_function_2();

// Hardware functions...
static void _input_sw_irq_handler(uint32_t events);


static cmt_msg_t _msg_be_initialized;

static uint32_t _last_rtc_update_ts; // ms timestamp of the last time we updated the RTC

static const msg_handler_entry_t _be_test = { MSG_BE_TEST, _handle_be_test };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _cmt_sm_tick_handler_entry = { MSG_CMT_SLEEP, _handle_cmt_sleep };
static const msg_handler_entry_t _input_sw_debnce_handler_entry = { MSG_INPUT_SW_DEBOUNCE, _handle_input_sw_debounce };
static const msg_handler_entry_t _panal_repeat_21ms_handler_entry = { MSG_PANEL_REPEAT_21MS, _handle_panel_repeat_21ms };
static const msg_handler_entry_t _stdio_char_ready_handler_entry = { MSG_STDIO_CHAR_READY, stdio_chars_read };
static const msg_handler_entry_t _switch_action_handler_entry = { MSG_SWITCH_ACTION, _handle_switch_action };
static const msg_handler_entry_t _switch_longpress_b1_handler_entry = { MSG_B1SW_LONGPRESS_DELAY, _handle_switch_longpress_delay };
static const msg_handler_entry_t _switch_longpress_b2_handler_entry = { MSG_B2SW_LONGPRESS_DELAY, _handle_switch_longpress_delay };
static const msg_handler_entry_t _ui_initialized_handler_entry = { MSG_UI_INITIALIZED, _handle_ui_initialized };

// For performance - put these in order that we expect to receive more often
static const msg_handler_entry_t* _be_handler_entries[] = {
    & _panal_repeat_21ms_handler_entry,
    & _cmt_sm_tick_handler_entry,
    & _panel_slowblnk_handler_entry,
    & _switch_action_handler_entry,
    & _switch_longpress_b1_handler_entry,
    & _switch_longpress_b2_handler_entry,
    & _stdio_char_ready_handler_entry,
    & _config_changed_handler_entry,
    & _input_sw_debnce_handler_entry,
    & _ui_initialized_handler_entry,
    & _be_test,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

static const idle_fn _be_idle_functions[] = {
    // Cast needed do to definition needed to avoid circular reference.
    (idle_fn)_be_idle_function_1,
    (idle_fn)_be_idle_function_2,
    (idle_fn)0, // Last entry must be a NULL
};

msg_loop_cntx_t be_msg_loop_cntx = {
    BE_CORE_NUM, // Back-end runs on Core 0
    _be_handler_entries,
    _be_idle_functions,
};

// ====================================================================
// Idle functions
//
// Something to do when there are no messages to process.
// (These are cycled through, so do one task.)
// ====================================================================

/**
 * @brief Do an actual check of the input switch to make sure we stay
 *        in sync with the actual state (not just relying on the
 *        interrupts).
 * @ingroup be
 */
static void _be_idle_function_1() {
    _input_sw_pressed = _input_sw_pressed && user_switch_pressed();
}

/**
 * @brief Do periodic system updates during idle time.
 * @ingroup be
 */
static void _be_idle_function_2() {
    uint32_t now = now_ms();
    if (_last_rtc_update_ts + ONE_HOUR_MS < now) {
        // Keep the RTC set correctly by making a NTP call.
        _last_rtc_update_ts = now;
        const config_sys_t* cfgsys = config_sys();
        network_update_rtc(cfgsys->tz_offset);
    }
}


// ====================================================================
// Message handler functions
// ====================================================================

static void _handle_be_test(cmt_msg_t* msg) {
    // Test `scheduled_msg_ms` error
    static int times = 1;
    static cmt_msg_t msg_time = { MSG_BE_TEST };

    uint64_t period = 60;

    if (debug_mode_enabled()) {
        uint64_t now = now_us();

        uint64_t last_time = msg->data.ts_us;
        int64_t error = ((now - last_time) - (period * 1000 * 1000));
        float error_per_ms = ((error * 1.0) / (period * 1000.0));
        info_printf(true, "\n%5.5d - Scheduled msg delay error us/ms:%5.2f\n", times, error_per_ms);
    }
    msg_time.data.ts_us = now_us(); // Get the 'next' -> 'last_time' fresh
    schedule_msg_in_ms((period * 1000), &msg_time);
    times++;
}

static void _handle_cmt_sleep(cmt_msg_t* msg) {
    cmt_handle_sleep(msg);
}

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    const config_t* cfg = config_current();
    // See if things we care about have changed...

    // Hold on to the new config
    _last_cfg = config_copy(_last_cfg, cfg);
}

static void _handle_input_sw_debounce(cmt_msg_t* msg) {
    _input_sw_pressed = user_switch_pressed(); // See if it's still pressed
    if (_input_sw_pressed) {
        cmt_msg_t msg = { MSG_INPUT_SW_PRESS };
        postUIMsgBlocking(&msg);
    }
}

static void _handle_panel_repeat_21ms(cmt_msg_t* msg) {
    // The panel repeat-21ms is a repetitive message that occurs every 21ms.
    // We use it to poll the switch banks if they are enabled.
    if (_ui_initialized) {
        curswitch_trigger_read();
    }
}

static void _handle_switch_action(cmt_msg_t* msg) {
    // Handle switch actions so we can detect a long press
    // and post a message for it.
    //
    // We keep track of one switch in each bank. We assume
    // that only one switch (per bank) can be pressed at
    // a time, so we only keep track of the last one pressed.
    //
    switch_bank_t bank = msg->data.sw_action.bank;
    switch_id_t sw_id = msg->data.sw_action.switch_id;
    bool pressed = msg->data.sw_action.pressed;
    switch (bank) {
        case SWBANK1:
            if (!pressed) {
                // Clear any long press in progress
                scheduled_msg_cancel(MSG_B1SW_LONGPRESS_DELAY);
                _bank1_sw_pressed = SW_NONE;
            }
            else {
                // Start a delay timer
                switch_action_data_t *sad = &_bank1_sw_longpress_msg.data.sw_action;
                _bank1_sw_pressed = sw_id;
                sad->bank = bank;
                sad->switch_id = sw_id;
                sad->pressed = true;
                sad->repeat = false;
                schedule_msg_in_ms(config_current()->long_press, &_bank1_sw_longpress_msg);
            }
            break;
        case SWBANK2:
            if (!pressed) {
                // Clear any long press in progress
                scheduled_msg_cancel(MSG_B2SW_LONGPRESS_DELAY);
                _bank2_sw_pressed = SW_NONE;
            }
            else {
                // Start a delay timer
                switch_action_data_t *sad = &_bank2_sw_longpress_msg.data.sw_action;
                _bank2_sw_pressed = sw_id;
                sad->bank = bank;
                sad->switch_id = sw_id;
                sad->pressed = true;
                sad->repeat = false;
                schedule_msg_in_ms(config_current()->long_press, &_bank2_sw_longpress_msg);
            }
            break;
    }
}

static void _handle_switch_longpress_delay(cmt_msg_t* msg) {
    // Handle the long press delay message to see if the switch is still pressed.
    switch_bank_t bank = msg->data.sw_action.bank;
    switch_id_t sw_id = msg->data.sw_action.switch_id;
    bool repeat = msg->data.sw_action.repeat;
    cmt_msg_t* slpmsg = NULL;
    switch (bank) {
        case SWBANK1:
            if (sw_id == _bank1_sw_pressed) {
                // Prepare to post another delay msg.
                slpmsg = &_bank1_sw_longpress_msg;
            }
            else {
                sw_id = SW_NONE;
            }
            break;
        case SWBANK2:
            if (sw_id == _bank2_sw_pressed) {
                // Prepare to post another delay msg.
                slpmsg = &_bank2_sw_longpress_msg;
            }
            else {
                sw_id = SW_NONE;
            }
            break;
        default:
            sw_id = SW_NONE;
            break;
    }
    if (sw_id != SW_NONE) {
        // Yes, the same switch is still pressed
        cmt_msg_t msg = { MSG_SWITCH_LONGPRESS };
        msg.data.sw_action.bank = bank;
        msg.data.sw_action.switch_id = sw_id;
        msg.data.sw_action.pressed = true;
        msg.data.sw_action.repeat = repeat;
        postBothMsgNoWait(&msg);
        // Schedule another delay
        switch_action_data_t* sad = &slpmsg->data.sw_action;
        sad->bank = bank;
        sad->switch_id = sw_id;
        sad->pressed = true;
        sad->repeat = true;
        uint16_t delay = (repeat ? SWITCH_REPEAT_MS : config_current()->long_press);
        schedule_msg_in_ms(delay, slpmsg);
    }
}

static void _handle_ui_initialized(cmt_msg_t* msg) {
    // The UI has reported that it is initialized.
    // Since we are responding to a message, it means we
    // are also initialized, so -
    //
    // Start things running.
    _ui_initialized = true;
}


// ====================================================================
// Hardware operational functions
// ====================================================================


void _gpio_irq_handler(uint gpio, uint32_t events) {
    switch (gpio) {
    case USER_INPUT_SW:
        _input_sw_irq_handler(events);
        break;
    }
}

static void _input_sw_irq_handler(uint32_t events) {
    // The user input switch and the infrared receiver B share the same GPIO.
    // The GPIO needs to be low for at least 80ms to be considered a button press.
    // Anything shorter is probably the IR, which is handled by PIO.
    if (events & GPIO_IRQ_EDGE_FALL) {
        // Delay to see if it is user input or an IR received.
        // Check to see if we have already scheduled a debounce message.
        if (!scheduled_message_exists(MSG_INPUT_SW_DEBOUNCE)) {
            schedule_msg_in_ms(80, &_input_sw_debounce_msg);
        }
    }
    if (events & GPIO_IRQ_EDGE_RISE) {
        // If we haven't recorded the input switch as pressed, this is probably the IR-B
        if (scheduled_message_exists(MSG_INPUT_SW_DEBOUNCE)) {
            scheduled_msg_cancel(MSG_INPUT_SW_DEBOUNCE);
        }
        if (_input_sw_pressed) {
            _input_sw_pressed = false;
            cmt_msg_t msg = { MSG_INPUT_SW_RELEASE };
            postUIMsgBlocking(&msg);
        }
    }
}


// ====================================================================
// Initialization functions
// ====================================================================


void be_module_init() {
    _input_sw_pressed = false;
    gpio_set_irq_enabled_with_callback(IRQ_INPUT_SW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &_gpio_irq_handler);
    _last_rtc_update_ts = 0;
    const config_t* cfg = config_current();
    _last_cfg = config_new(cfg);
    panel_type_t panel_type = config_sys()->panel_type;
    panel_module_init(panel_type);

    // Done with the Backend Initialization - Let the UI know.
    _msg_be_initialized.id = MSG_BE_INITIALIZED;
    postUIMsgBlocking(&_msg_be_initialized);
    // Post a TEST to ourself in case we have any tests set up.
    cmt_msg_t msg = { MSG_BE_TEST };
    postBEMsgNoWait(&msg);
}

void start_be() {
    static bool _started = false;
    // Make sure we aren't already started and that we are being called from core-0.
    assert(!_started && 0 == get_core_num());
    _started = true;
    message_loop(&be_msg_loop_cntx);
}
