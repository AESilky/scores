/**
 * scores User Interface - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "system_defs.h"
#include "ui.h"
#include "ui_disp.h"
#include "ui_term.h"

#include "board.h"
#include "cmd/cmd.h"
#include "cmt/cmt.h"
#include "cmt/core1_main.h"
#include "cmt/multicore.h"
#include "config/config.h"
#include "curswitch/curswitch.h"
#include "rc/rc.h"
#include "scorekeeper/sk_app.h"
#include "scorekeeper/sk_tod.h"
#include "setup/setup.h"
#include "util/util.h"

#include "hardware/rtc.h"

#include <stdlib.h>
#include <string.h>

#define _UI_STATUS_PULSE_PERIOD 7001

typedef enum _UI_App_ID_ {
    APP_NONE    = 0,
    APP_SCORES  = 1,
    APP_SETUP   = 2,
} ui_app_id_t;

static ui_app_id_t _app_active;
static bool _initialized = false;

// Internal, non message handler, function declarations
void _ui_init_terminal_shell();
void _ui_setup_app_done();

// Message handler functions...
static void _handle_be_initialized(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_init_terminal(cmt_msg_t* msg);
static void _handle_input_switch_pressed(cmt_msg_t* msg);
static void _handle_input_switch_released(cmt_msg_t* msg);
static void _handle_rc_action(cmt_msg_t* msg);
static void _handle_rc_longpress(cmt_msg_t* msg);
static void _handle_rc_value_entered(cmt_msg_t* msg);
static void _handle_switch_action(cmt_msg_t* msg);
static void _handle_switch_longpress(cmt_msg_t* msg);

static void _ui_idle_function_1();

static cmt_msg_t _msg_ui_initialized;

static const msg_handler_entry_t _be_initialized_handler_entry = { MSG_BE_INITIALIZED, _handle_be_initialized };
static const msg_handler_entry_t _cmd_init_terminal_handler_entry = { MSG_CMD_INIT_TERMINAL, _handle_init_terminal };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _input_sw_pressed_handler_entry = { MSG_INPUT_SW_PRESS, _handle_input_switch_pressed };
static const msg_handler_entry_t _input_sw_released_handler_entry = { MSG_INPUT_SW_RELEASE, _handle_input_switch_released };
static const msg_handler_entry_t _rc_action_handler_entry = { MSG_RC_ACTION, _handle_rc_action };
static const msg_handler_entry_t _rc_longpress_handler_entry = { MSG_RC_LONGPRESS, _handle_rc_longpress };
static const msg_handler_entry_t _rc_value_handler_entry = { MSG_RC_VALUE_ENTERED, _handle_rc_value_entered };
static const msg_handler_entry_t _switch_action_handler_entry = { MSG_SWITCH_ACTION, _handle_switch_action };
static const msg_handler_entry_t _switch_longpress_handler_entry = { MSG_SWITCH_LONGPRESS, _handle_switch_longpress };

/**
 * @brief List of handler entries.
 * @ingroup ui
 *
 * For performance, put these in the order that we expect to receive the most (most -> least).
 *
 */
static const msg_handler_entry_t* _handler_entries[] = {
    & cmt_sm_tick_handler_entry,
    &_sk_tod_update_handler_entry,
    &_rc_action_handler_entry,
    &_switch_action_handler_entry,
    &_rc_longpress_handler_entry,
    &_switch_longpress_handler_entry,
    &_rc_value_handler_entry,
    &_input_sw_pressed_handler_entry,
    &_input_sw_released_handler_entry,
    &_config_changed_handler_entry,
    &_cmd_init_terminal_handler_entry,
    &_be_initialized_handler_entry,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

static const idle_fn _ui_idle_functions[] = {
    (idle_fn)_ui_idle_function_1,
    (idle_fn)0, // Last entry must be a NULL
};

msg_loop_cntx_t ui_msg_loop_cntx = {
    UI_CORE_NUM, // UI runs on Core 1
    _handler_entries,
    _ui_idle_functions,
};


// ============================================
// Idle functions
// ============================================

static void _ui_idle_function_1() {
    // Something to do when there are no messages to process.
}


// ============================================
// Message handler functions
// ============================================

static void _handle_be_initialized(cmt_msg_t* msg) {
    // The Backend has reported that it is initialized.
    // Since we are responding to a message, it means we
    // are also initialized, so -
}

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    //const config_t* cfg = config_current();
}

/**
 * @brief Message handler for MSG_INIT_TERMINAL
 * @ingroup ui
 *
 * Init/re-init the terminal. This is typically received by a user requesting
 * that the terminal be re-initialized/refreshed. For example if they connect
 * a terminal after scores is already up and running.
 *
 * @param msg Nothing important in the message.
 */
static void _handle_init_terminal(cmt_msg_t* msg) {
    _ui_init_terminal_shell();
}

/**
 * @brief Message handler for MSG_INPUT_SW_PRESS
 * @ingroup ui
 *
 * The BE has determined that the input switch has been pressed.
 *
 * @param msg Nothing in the data of this message.
 */
static void _handle_input_switch_pressed(cmt_msg_t* msg) {
    // Check that it's still pressed...
    if (user_switch_pressed()) {
        debug_printf(false, "Input switch pressed\n");
    }
}

/**
 * @brief Message handler for MSG_INPUT_SW_RELEASE
 * @ingroup ui
 *
 * The BE has determined that the input switch has been released.
 *
 * @param msg Nothing in the data of this message.
 */
static void _handle_input_switch_released(cmt_msg_t* msg) {
    debug_printf(false, "Input switch released\n");
}

/**
 * @brief Message handler for MSG_RC_ACTION
 * @ingroup ui
 *
 * The Remote Control processing determined that a key was pressed
 *
 * @param msg Contains a rc_action_data_t structure with information
 *              about the action.
 */
static void _handle_rc_action(cmt_msg_t* msg) {
    rc_action_data_t action = msg->data.rc_action;
    rc_vcode_t code = action.code;
    bool repeat = msg->data.rc_action.repeat;
    // Don't do anything if it is a repeat. We use the LONGPRESS message for that.
    if (!repeat) {
        info_printf(false, "Remote: %d\n", code);
        switch (_app_active) {
            case APP_SCORES:
                sk_app_rc_action(action, false);
                break;
            case APP_SETUP:
                setup_app_rc_action(action, false);
                break;
            case APP_NONE:
                // No application active. Nothing to do.
                break;
        }
    }
}

/**
 * @brief Message handler for MSG_RC_LONGPRESS
 * @ingroup ui
 *
 * A Remote Control button was long-pressed.
 *
 * @param msg Contains a rc_action_data_t structure with information
 *              about the action.
 */
static void _handle_rc_longpress(cmt_msg_t* msg) {
    rc_action_data_t action = msg->data.rc_action;
    rc_vcode_t code = action.code;
    bool repeat = msg->data.rc_action.repeat;
    const char* repeatstr = (repeat ? " repeat" : "");
    info_printf(false, "Remote: %d Long Press%s\n", code, repeatstr);
    switch (_app_active) {
        case APP_SCORES:
            // If this is a LONG-PRESS+REPEAT of the MENU button, enter the Setup App.
            if (repeat && code == RC_MENU) {
                if (setup_app_run(_ui_setup_app_done)) {
                    _app_active = APP_SETUP;
                    return;
                }
            }
            sk_app_rc_action(action, true);
            break;
        case APP_SETUP:
            setup_app_rc_action(action, true);
            break;
        case APP_NONE:
            // No application active. Nothing to do.
            break;
    }
}

/**
 * @brief Message handler for MSG_RC_VALUE_ENTERED
 * @ingroup ui
 *
 * The Remote Control processing determined that a numeric was entered
 *
 * @param msg Contains a rc_entry_data_t structure with information
 *              about the action.
 */
static void _handle_rc_value_entered(cmt_msg_t* msg) {
    rc_vcode_t code = msg->data.rc_entry.code;
    int value = msg->data.rc_entry.value;
    int divisor = msg->data.rc_entry.divisor;
    info_printf(false, "Remote value entered: %d  Divisor: %d  Terminator: %d\n", value, divisor, code);
    switch (_app_active) {
        case APP_SCORES:
            sk_app_rc_entry(msg->data.rc_entry);
            break;
        case APP_SETUP:
            setup_app_rc_entry(msg->data.rc_entry);
            break;
        case APP_NONE:
            // No application active. Nothing to do.
            break;
    }
}

/**
 * @brief Message handler for MSG_SWITCH_ACTION
 * @ingroup ui
 *
 * The Cursor Switch processing determined that a switch was pushed
 * or released.
 *
 * @param msg Contains a switch_action_data_t structure with information
 *              about the action.
 */
static void _handle_switch_action(cmt_msg_t* msg) {
    switch_bank_t bank = msg->data.sw_action.bank;
    switch_id_t sw_id = msg->data.sw_action.switch_id;
    bool pressed = msg->data.sw_action.pressed;
    char* state;
    const char* swname = curswitch_shortname_for_swid(sw_id);
    state = (pressed ? "Pressed" : "Released");
    info_printf(false, "Bank%d %s %s\n", bank, swname, state);
    switch (_app_active) {
        case APP_SCORES:
            sk_app_switch_action(bank, sw_id, pressed, false, false);
            break;
        case APP_SETUP:
            setup_app_switch_action(bank, sw_id, pressed, false, false);
            break;
        case APP_NONE:
            // No application active. Nothing to do.
            break;
    }
}

/**
 * @brief Message handler for MSG_SWITCH_LONGPRESS
 * @ingroup ui
 *
 * A Cursor Switch was long-pressed.
 *
 * @param msg Contains a switch_action_data_t structure with information
 *              about the action.
 */
static void _handle_switch_longpress(cmt_msg_t* msg) {
    switch_bank_t bank = msg->data.sw_action.bank;
    switch_id_t sw_id = msg->data.sw_action.switch_id;
    bool repeat = msg->data.sw_action.repeat;
    const char* swname = curswitch_shortname_for_swid(sw_id);
    const char* repeatstr = (repeat ? " repeat" : "");
    debug_printf(false, "Bank%d %s Long Press%s\n", bank, swname, repeatstr);
    switch (_app_active) {
        case APP_SCORES:
            sk_app_switch_action(bank, sw_id, true, true, repeat);
            break;
        case APP_SETUP:
            setup_app_switch_action(bank, sw_id, true, true, repeat);
            break;
        case APP_NONE:
            // No application active. Nothing to do.
            break;
    }
}

// ============================================
// Internal functions
// ============================================

/**
 * @brief Callback for the Setup app when done.
 * @ingroup ui
 */
void _ui_setup_app_done(void) {
    _app_active = APP_SCORES;
    sk_app_refresh();
}

void _ui_init_terminal_shell() {
    ui_term_build();
    cmd_module_init();
}


/////////////////////////////////////////////////////////////////////
// Command processor support
/////////////////////////////////////////////////////////////////////
//
static int _remote_code_cmd(int argc, char** argv, const char* unparsed) {
    if (argc > 1) {
        // The args are the remote code to simulate. Valid codes are -255 to 0 to 255.
        // Negative codes indicate a repeat of the positive code.
        int i = 1;
        // Check the values entered
        while (i < argc) {
            int code = atoi(argv[i]);
            if (code < -255 || code > 255) {
                ui_term_printf("Remote code value %d must be -255 to 255 (negative indicates a repeat)\n", i);
                return (-1);
            }
            i++;
        }
        // Process the values entered
        i = 1;
        while (i < argc) {
            int code = atoi(argv[i++]);
            rc_handle_code(code);
        }
    }
    else {
        ui_term_printf("No values entered.\n");
        return (-1);
    }

    return (0);
}


const cmd_handler_entry_t cmd_ui_remote_code_entry = {
    _remote_code_cmd,
    1,
    "rc",
    "code",
    "Simulate receiving a code from the remote.",
};


// ============================================
// Public functions
// ============================================

/**
 * @brief Start the UI - The Core 1 message loop.
 */
void start_ui(void) {
    static bool _started = false;
    // Make sure we aren't already started and that we are being called from core-0.
    assert(!_started && 0 == get_core_num());
    _started = true;

    start_core1(); // The Core-1 main starts the UI
}

bool ui_initialized() {
    return _initialized;
}

/**
 * @brief Initialize the User Interface (now that the message loop is running).
 */
void ui_module_init() {
    _app_active = APP_NONE;
    ui_disp_build();
    _ui_init_terminal_shell();
    // Initialize the Setup functionality
    setup_module_init();
    // Initialize the Score Keeper app
    sk_app_module_init();
    // Start out in the Scorekeeper functionality (switches to Setup at times)
    _app_active = APP_SCORES;

    // Let the Backend know that we are initialized
    _initialized = true;
    _msg_ui_initialized.id = MSG_UI_INITIALIZED;
    postBEMsgBlocking(&_msg_ui_initialized);
}
