/**
 * Remote control handling for Scorekeeper.
 *
 * Copyright 2023-24 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_remote_ctrl.h"

#include "scorekeeper/scorekeeper.h"

void ui_rc_decrease_value(uint8_t code, bool repeat) {
    // TODO: Handle repeat
    switch (code) {
        case RC_VOL_DOWN: // Team A
            scorekeeper_decrease_value(SKVALUE_A);
            break;
        case RC_CH_DOWN: // Team B
            scorekeeper_decrease_value(SKVALUE_B);
            break;
        case RC_PAGE_DOWN: // PT
            scorekeeper_decrease_value(SKVALUE_C);
            break;
    }
}

void ui_rc_display_blank(uint8_t code, bool repeat) {
    scorekeeper_mode_set(SKMODE_BLANK);
}

void ui_rc_display_scores(uint8_t code, bool repeat) {
    scorekeeper_mode_set(SKMODE_SCORES);
}

void ui_rc_display_tod(uint8_t code, bool repeat) {
    scorekeeper_mode_set(SKMODE_TOD);
}

void ui_rc_increase_value(uint8_t code, bool repeat) {
    // TODO: Handle repeat
    switch (code) {
        case RC_VOL_UP: // Team A
            scorekeeper_increase_value(SKVALUE_A);
            break;
        case RC_CH_UP: // Team B
            scorekeeper_increase_value(SKVALUE_B);
            break;
        case RC_PAGE_UP: // PT
            scorekeeper_increase_value(SKVALUE_C);
            break;
    }
}

void ui_rc_register_handlers(void) {
    // Register our handlers
    rc_register_handler(RC_VOL_UP, ui_rc_increase_value);
    rc_register_handler(RC_CH_UP, ui_rc_increase_value);
    rc_register_handler(RC_PAGE_UP, ui_rc_increase_value);
    rc_register_handler(RC_VOL_DOWN, ui_rc_decrease_value);
    rc_register_handler(RC_CH_DOWN, ui_rc_decrease_value);
    rc_register_handler(RC_PAGE_DOWN, ui_rc_decrease_value);
    rc_register_handler(RC_A, ui_rc_display_scores);
    rc_register_handler(RC_B, ui_rc_display_blank);
    rc_register_handler(RC_D, ui_rc_display_tod);
}
