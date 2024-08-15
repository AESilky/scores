/**
 * Scorekeeper application functionality.
 *
 *
 * Copyright 2023-24 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "sk_app.h"
#include "scorekeeper.h"

#include "board.h"
#include "rc/rc.h"

void sk_app_rc_action(rc_action_data_t action) {
    // If the RC is collecting a value, then wait for the value...
    if (!rc_is_collecting_value() && !action.repeat) {
        rc_vcode_t code = action.code;
        switch (code) {
            case RC_VOL_UP:
                scorekeeper_add_value(SKVALUE_A, 1);
                break;
            case RC_VOL_DOWN:
                scorekeeper_add_value(SKVALUE_A, -1);
                break;
            case RC_CH_UP:
                scorekeeper_add_value(SKVALUE_B, 1);
                break;
            case RC_CH_DOWN:
                scorekeeper_add_value(SKVALUE_B, -1);
                break;
            default:
                beep();
                break;
        }
    }
}

void sk_app_rc_entry(rc_value_entry_t entry) {
    // A number was entered, adjust the score based on the entry code (the ending button)
    rc_vcode_t code = entry.code;
    int value = entry.value;
    //int divisor = entry.divisor;
    //float fv = value / divisor;
    switch (code) {
        case RC_VOL_UP:
            scorekeeper_add_value(SKVALUE_A, value);
            break;
        case RC_VOL_DOWN:
            scorekeeper_add_value(SKVALUE_A, -1 * value);
            break;
        case RC_CH_UP:
            scorekeeper_add_value(SKVALUE_B, value);
            break;
        case RC_CH_DOWN:
            scorekeeper_add_value(SKVALUE_B, -1 * value);
            break;
        case RC_MENU_3BAR:
            scorekeeper_set_value(SKVALUE_B, value);
            break;
        case RC_EXIT:
            scorekeeper_set_value(SKVALUE_B, value);
            break;
        default:
            beep_long();
            break;
    }
    rc_value_collecting_reset();
}

void sk_app_switch_action(switch_bank_t bank, switch_id_t sw_id, bool pressed, bool long_press, bool repeat) {
    if (!pressed) {
        return;
    }
    // Handle the switches for keeping score
    int sv;
    // Bank1 is for the A score, Bank2 for B
    sk_value_ctrl_t vctrl = (bank == SWBANK1 ? SKVALUE_A : SKVALUE_B);

    // If this is a long_press of SW_ENTER, but not a repeat, set the score to 11
    if (long_press && !repeat) {
        int current_score = scorekeeper_get_value(vctrl);
        if (current_score > 11) {
            scorekeeper_set_value(vctrl, 11);
        }
        return;
    }
    // If this is a repeat long_press of SW_ENTER, set the score to 0
    if (long_press && repeat) {
        scorekeeper_set_value(vctrl, 0);
        return;
    }

    // Get a value to change the score by based on the switch
    switch (sw_id) {
        case SW_LEFT:
            // Increase score by 3
            sv = 3;
            break;
        case SW_HOME:
            // Increase score by 2
            sv = 2;
            break;
        case SW_RIGHT:
            // Increase score by 1
            sv = 1;
            break;
        case SW_ENTER:
            // Decreate score by 1
            sv = -1;
            break;
        default:
            // We don't expect the other switches to be installed
            // but we'll handle added values
            if (sw_id == SW_UP) {
                sv = 5;
            }
            else if (sw_id == SW_DOWN) {
                sv = -5;
            }
            else {
                sv = 0;
            }
            break;
    }
    scorekeeper_add_value(vctrl, sv);
}

void sk_app_module_init() {
    scorekeeper_module_init();
}
