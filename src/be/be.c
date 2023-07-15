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
#include "net/net.h"
#include "scorekeeper/scorekeeper.h"
#include "util/util.h"

#include "hardware/rtc.h"

#define _BE_STATUS_PULSE_PERIOD 6999

static config_t* _last_cfg;

// Message handler functions...
static void _handle_be_test(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_cmt_sleep(cmt_msg_t* msg);
static void _handle_send_be_status(cmt_msg_t* msg);
static void _handle_ui_initialized(cmt_msg_t* msg);

// Idle functions...
static void _be_idle_function_1();
static void _be_idle_function_2();
static void _be_idle_function_3();

static cmt_msg_t _msg_be_send_status;
static cmt_msg_t _msg_be_initialized;

static uint32_t _last_rtc_update_ts; // ms timestamp of the last time we updated the RTC
static uint32_t _last_status_update_ts; // ms timestamp of last status update

static const msg_handler_entry_t _be_test = { MSG_BE_TEST, _handle_be_test };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _cmt_sm_tick_handler_entry = { MSG_CMT_SLEEP, _handle_cmt_sleep };
static const msg_handler_entry_t _send_be_status_handler_entry = { MSG_SEND_BE_STATUS, _handle_send_be_status };
static const msg_handler_entry_t _ui_initialized_handler_entry = { MSG_UI_INITIALIZED, _handle_ui_initialized };

// For performance - put these in order that we expect to receive more often
static const msg_handler_entry_t* _be_handler_entries[] = {
    & _cmt_sm_tick_handler_entry,
    & _send_be_status_handler_entry,
    & _config_changed_handler_entry,
    & _ui_initialized_handler_entry,
    & _be_test,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

static const idle_fn _be_idle_functions[] = {
    // Cast needed do to definition needed to avoid circular reference.
    (idle_fn)_be_idle_function_1,
    (idle_fn)_be_idle_function_2,
    (idle_fn)_be_idle_function_3,
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

static void _be_idle_function_1() {
}

static void _be_idle_function_2() {
    uint32_t now = now_ms();
    if (_last_rtc_update_ts + ONE_HOUR_MS < now) {
        // Keep the RTC set correctly by making a NTP call.
        _last_rtc_update_ts = now;
        const config_sys_t* cfgsys = config_sys();
        network_update_rtc(cfgsys->tz_offset);
    }
}

static void _be_idle_function_3() {
    uint32_t now = now_ms();
    if (_last_status_update_ts + _BE_STATUS_PULSE_PERIOD < now) {
        // Post update status message
        _msg_be_send_status.id = MSG_SEND_BE_STATUS;
        postBEMsgNoWait(&_msg_be_send_status); // Don't wait. We will do it again in a bit.
        _last_status_update_ts = now;
    }
}


// ====================================================================
// Message handler functions
// ====================================================================

static void _handle_be_test(cmt_msg_t* msg) {
    // Test `scheduled_msg_ms` error
    static int times = 1;
    static cmt_msg_t msg_time = { MSG_BE_TEST };
    static uint64_t first_t = 0;

    uint64_t period = 60;

    if (debug_mode_enabled()) {
        uint64_t now = now_us();
        if (first_t == 0) { first_t = now; }

        uint64_t last_time = msg->data.ts_us;
        int64_t error = ((now - last_time) - (period * 1000 * 1000));
        int64_t total_error = (now - (first_t + (times * (period * 1000 * 1000))));
        float error_per_ms = ((error * 1.0) / (period * 1000.0));
        info_printf(true, "\n%5d - Error us/ms:%5.2f  Avg:%5d\n", times, error_per_ms, (total_error / (times * period)));
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

static void _handle_send_be_status(cmt_msg_t* msg) {
    // Update our status
}

static void _handle_ui_initialized(cmt_msg_t* msg) {
    // The UI has reported that it is initialized.
    // Since we are responding to a message, it means we
    // are also initialized, so -
    //
    // Start things running.
    scorekeeper_update_panel_scores();
}

void be_module_init() {
    _last_rtc_update_ts = 0;
    const config_t* cfg = config_current();
    _last_cfg = config_new(cfg);
    scorekeeper_module_init();

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
