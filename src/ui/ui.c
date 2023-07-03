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

#include "board.h"
#include "cmd.h"
#include "cmt.h"
#include "config.h"
#include "core1_main.h"
#include "board.h"
#include "multicore.h"
#include "util.h"
#include "ui_disp.h"
#include "ui_term.h"

#include "hardware/rtc.h"

#include <stdlib.h>
#include <string.h>

#define _UI_STATUS_PULSE_PERIOD 7001

// Internal, non message handler, function declarations
static void _ui_init_terminal_shell();
static void _user_input_sw_irq_handler(uint gpio, uint32_t events);

// Message handler functions...
static void _handle_be_initialized(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_init_terminal(cmt_msg_t* msg);

static void _ui_idle_function_1();

static cmt_msg_t _msg_ui_cmd_start;
static cmt_msg_t _msg_ui_initialized;

static const msg_handler_entry_t _be_initialized_handler_entry = { MSG_BE_INITIALIZED, _handle_be_initialized };
static const msg_handler_entry_t _cmd_key_pressed_handler_entry = { MSG_CMD_KEY_PRESSED, cmd_attn_handler };
static const msg_handler_entry_t _cmd_init_terminal_handler_entry = { MSG_CMD_INIT_TERMINAL, _handle_init_terminal };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _input_char_ready_handler_entry = { MSG_INPUT_CHAR_READY, _ui_term_handle_input_char_ready };

/**
 * @brief List of handler entries.
 * @ingroup ui
 *
 * For performance, put these in the order that we expect to receive the most (most -> least).
 *
 */
static const msg_handler_entry_t* _handler_entries[] = {
    &_input_char_ready_handler_entry,
    &_cmd_key_pressed_handler_entry,
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
    //
    _msg_ui_cmd_start.id = MSG_CMD_KEY_PRESSED;
    _msg_ui_cmd_start.data.c = CMD_WAKEUP_CHAR;
    postUIMsgBlocking(&_msg_ui_cmd_start);
}

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    const config_t* cfg = config_current();
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


// ============================================
// Internal functions
// ============================================

static void _ui_init_terminal_shell() {
    ui_term_build();
    cmd_module_init();
}

void _ui_gpio_irq_handler(uint gpio, uint32_t events) {
    switch (gpio) {
        case USER_INPUT_SW:
            _user_input_sw_irq_handler(gpio, events);
            break;
    }
}

static void _user_input_sw_irq_handler(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        debug_printf(false, "User Input Switch pressed.\n");
    }
    if (events & GPIO_IRQ_EDGE_RISE) {
        //printf("re switch released\n");
    }
}


// ============================================
// Public functions
// ============================================

void start_ui(void) {
    static bool _started = false;
    // Make sure we aren't already started and that we are being called from core-0.
    assert(!_started && 0 == get_core_num());
    _started = true;
    start_core1(); // The Core-1 main starts the UI
}

void ui_module_init() {
    gpio_set_irq_enabled_with_callback(IRQ_USER_INPUT_SW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &_ui_gpio_irq_handler);

    ui_disp_build();
    _ui_init_terminal_shell();

    // Let the Backend know that we are initialized
    _msg_ui_initialized.id = MSG_UI_INITIALIZED;
    postBEMsgBlocking(&_msg_ui_initialized);
}
