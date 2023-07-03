/**
 * scores Debugging flags and utilities.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */
#include "debug_support.h"
#include "cmt.h"
#include "ui_term.h"
#include "util.h"

volatile uint16_t debugging_flags = 0;
static bool _debug_mode_enabled = false;

static int _debug_support_cmd_debug(int argc, char** argv, const char* unparsed) {
    if (argc > 2) {
        // We only take a single argument.
        cmd_help_display(&cmd_debug_support_entry, HELP_DISP_USAGE);
        return (-1);
    }
    else if (argc > 1) {
        // Argument is bool (ON/TRUE/YES/1 | <anything-else>) to set flag
        bool b = bool_from_str(argv[1]);
        debug_mode_enable(b);
    }
    ui_term_printf("Debug: %s\n", (debug_mode_enabled() ? "ON" : "OFF"));

    return (0);
}


const cmd_handler_entry_t cmd_debug_support_entry = {
    _debug_support_cmd_debug,
    2,
    ".debug",
    "[ON|OFF]",
    "Set/reset debug flag.",
};


bool debug_mode_enabled() {
    return _debug_mode_enabled;
}

bool debug_mode_enable(bool on) {
    bool temp = _debug_mode_enabled;
    _debug_mode_enabled = on;
    if (on != temp && cmt_message_loops_running()) {
        cmt_msg_t msg = { MSG_DEBUG_CHANGED };
        msg.data.debug = _debug_mode_enabled;
        postBothMsgNoWait(&msg);
    }
    return (temp != on);
}

