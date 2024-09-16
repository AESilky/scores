/**
 * @brief Application Setup functionality.
 * @ingroup setup
 *
 * This provides application level setup via the remote control and/or switch banks and the
 * oled screen display.
 *
 * Copyright 2024 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "setup.h"

#include "board.h"
#include "config/config.h"
#include "display/oled1106_spi/display_oled1106.h"
#include "rc/rc.h"
#include "util/util.h"

#include <stdio.h>

static bool _app_running;
static setup_callback_fn _cb;

static const char* _items[] = {
    "Line 1",
    "Line 2"
};

void _su_exit() {
    _app_running = false;
    if (_cb) {
        setup_callback_fn cb = _cb;
        _cb = (setup_callback_fn)0;
        cb();
    }
}

void _su_show() {
    disp_clear(true);
    disp_string(0, 0, "    SETUP     ", true, true);
    for (int i = 0; i < ARRAY_ELEMENT_COUNT(_items); i++) {
        disp_string(i+1, 0, _items[i], false, true);
    }
}

bool setup_app_run(setup_callback_fn cb) {
    if (_app_running) {
        return false;
    }
    _cb = cb;
    _app_running = true;
    _su_show();

    return true;
}

void setup_app_rc_action(rc_action_data_t action, bool longpress) {
    // If the RC is collecting a value, then wait for the value...
    if (!rc_is_collecting_value() && !action.repeat) {
        rc_vcode_t code = action.code;
        switch (code) {
            case RC_CURSOR_UP:
                break;
            case RC_CURSOR_DOWN:
                break;
            case RC_CURSOR_LEFT:
                break;
            case RC_CURSOR_RIGHT:
                break;
            case RC_OK:
                break;
            case RC_EXIT:
                _su_exit();
                return;
            default:
                beep();
                break;
        }
    }
}

void setup_app_rc_entry(rc_value_entry_t entry) {
    // A number was entered, adjust the score based on the entry code (the ending button)
    rc_vcode_t code = entry.code;
    //int value = entry.value;
    //int divisor = entry.divisor;
    //float fv = value / divisor;
    switch (code) {
        case RC_EXIT:
            _su_exit();
            break;
        default:
            beep_long();
            break;
    }
    rc_value_collecting_reset();
}

void setup_app_switch_action(switch_bank_t bank, switch_id_t sw_id, bool pressed, bool long_press, bool repeat) {
    if (!pressed) {
        // We don't do anything with RELEASE
        return;
    }
    // Handle the switches for navigating the setup menu and making selections.

    // If this is a repeat long_press of SW_ENTER, exit Setup.
    if (sw_id == SW_ENTER && long_press && repeat) {
        _su_exit();
        return;
    }

    //
    switch (sw_id) {
        case SW_LEFT:
            break;
        case SW_HOME:
            break;
        case SW_RIGHT:
            break;
        case SW_ENTER:
            break;
        case SW_UP:
            break;
        case SW_DOWN:
            break;
        default:
            break;
    }
}


void setup_module_init(void) {
    _app_running = false;
    _cb = NULL;
}
