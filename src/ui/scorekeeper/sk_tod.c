/**
 * Time of day display on the panel.
 *
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "sk_tod.h"
#include "panel/panel.h"
#include "panel/segments7/segments7.h"
#include "sk_screen.h"
#include "ui/ui_term.h"
#include "util/util.h"

#include "hardware/rtc.h"

/////////////////////////////////////////////////////////////////////
// Internal function declarations
/////////////////////////////////////////////////////////////////////
//
void _update_sk_tod();


/////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////
//
const msg_handler_entry_t _sk_tod_update_handler_entry = { MSG_PANEL_TOD_UPDATE, _sk_tod_update_handler };
const cmt_msg_t _tod_update_msg = { MSG_PANEL_TOD_UPDATE };

bool _enabled;
uint8_t _indicators;


/////////////////////////////////////////////////////////////////////
// Internal functions
/////////////////////////////////////////////////////////////////////
//
void _sk_tod_update_handler(cmt_msg_t* msg) {
    // Nothing important in the message.
    _update_sk_tod();
}

void _update_sk_tod() {
    static uint8_t update_ind_sec = 0;
    if (_enabled) {
        // Get the current time of day
        digsegs_t buf[2];
        datetime_t t;
        rtc_get_datetime(&t);
        // Update the panel
        dig2_int_b(buf, t.hour);
        panel_A_set(buf);
        dig2_int(buf, t.min);
        panel_B_set(buf);
        dig2_int(buf, t.sec);
        panel_C_set(buf);
        // Update the screen
        int8_t hour = (t.hour > 12 ? t.hour - 12 : t.hour);
        skscrn_A_set(hour);
        skscrn_B_set(t.min);
        skscrn_PT_set(t.sec);
        // Update the indicators
        if (t.sec == 0) {
            _indicators = 0xFF; // All on
            update_ind_sec = 0;
        }
        else {
            if (t.sec % 6 == 0 && t.sec != update_ind_sec) {
                update_ind_sec = t.sec;
                _indicators = (_indicators << 1);
            }
        }
        panel_IND_set(_indicators);
        skscrn_IND_set(_indicators);
        // Schedule us to run again
        schedule_core1_msg_in_ms(100, &_tod_update_msg); // Run again in 100ms
    }
    else {
        // Remove any pending scheduled message
        scheduled_msg_cancel(MSG_PANEL_TOD_UPDATE);
    }
}


/////////////////////////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////////////////////////
//
void sk_tod_enable(bool enable) {
    _enabled = enable;
    // Wake up display of the time of day.
    _update_sk_tod();
}

void sk_tod_module_init() {
    _enabled = false;
    _indicators = 0;
}

