/**
 * Scorekeeper functionality.
 *
 *
 * Copyright 2023-24 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "scorekeeper.h"
#include "sk_screen.h"
#include "sk_tod.h"

#include "config/config.h"
#include "panel/panel.h"
#include "panel/segments7/segments7.h"

#include "rc/rc.h"

#include "ui/ui_term.h"

#include <stdlib.h>

/////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////
//
static sk_content_mode_t _content_mode;
static sk_output_mode_t _output_mode;

static int8_t _score_a;
static int8_t _score_b;
static int8_t _value_c;
static uint8_t _indicators;

static bool _display_on_panel;
static bool _display_on_screen;

/////////////////////////////////////////////////////////////////////
// Internal function declarations
/////////////////////////////////////////////////////////////////////
//
static void _update_a_score();
static void _update_b_score();


/////////////////////////////////////////////////////////////////////
// Internal functions
/////////////////////////////////////////////////////////////////////
//
static void _clear_score() {
    _score_a = 0;
    _score_b = 0;
    if (_content_mode == SKMODE_SCORES) {
        _update_a_score();
        _update_b_score();
    }
}

static void _update_a_score() {
    if (_content_mode == SKMODE_SCORES) {
        if (_display_on_panel) {
            switch (_output_mode) {
                case SK_NUMERIC_MODE:
                    digsegs_t buf[2];
                    dig2_int(buf, _score_a);
                    panel_A_set(buf);
                    break;
                case SK_LINEAR_MODE:
                    linedots_t linedots = panel_linedots_for_value(_score_a);
                    panel_LinearA_set(linedots);
                    break;
            }
        }
        if (_display_on_screen) {
            skscrn_A_set(_score_a);
        }
    }
}

static void _update_b_score() {
    if (_content_mode == SKMODE_SCORES) {
        if (_display_on_panel) {
            switch (_output_mode) {
                case SK_NUMERIC_MODE:
                    digsegs_t buf[2];
                    dig2_int(buf, _score_b);
                    panel_B_set(buf);
                    break;
                case SK_LINEAR_MODE:
                    linedots_t linedots = panel_linedots_for_value(_score_b);
                    panel_LinearB_set(linedots);
                    break;
            }
        }
        if (_display_on_screen) {
            skscrn_B_set(_score_b);
        }
    }
}

static void _update_c_value() {
    if (_content_mode == SKMODE_SCORES) {
        if (_display_on_panel && _output_mode == SK_NUMERIC_MODE) {
            digsegs_t buf[2];
            dig2_int(buf, _value_c);
            panel_C_set(buf);
        }
        if (_display_on_screen && _output_mode == SK_NUMERIC_MODE) {
            skscrn_PT_set(_value_c);
        }
    }
}

static void _update_indicators() {
    if (_content_mode == SKMODE_SCORES) {
        if (_display_on_panel) {
            panel_IND_set(_indicators);
        }
        if (_display_on_screen) {
            skscrn_IND_set(_indicators);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////////////////////////
//

void scorekeeper_add_value(sk_value_ctrl_t vctrl, int v) {
    int8_t *vc;
    switch (vctrl) {
        case SKVALUE_A:
            vc = &_score_a;
            break;
        case SKVALUE_B:
            vc = &_score_b;
            break;
        case SKVALUE_C:
            vc = &_value_c;
            break;
    }
    int8_t value = *vc;
    value += v;
    switch (_output_mode) {
        case SK_LINEAR_MODE:
            if (value < 0) {
                value = 0;
            }
            if (value > SK_LINEAR_MAX_SCORE) {
                value = SK_LINEAR_MAX_SCORE;
            }
            break;
        case SK_NUMERIC_MODE:
            if (value < 0) {
                value = SK_NUMERIC_MAX_SCORE;
            }
            if (value > SK_NUMERIC_MAX_SCORE) {
                value = 0;
            }
            break;
        default:
            return;
    }
    *vc = value;
    scorekeeper_update_display();
}

void scorekeeper_clear_all() {
    _value_c = 0;
    _indicators = 0;
    _clear_score();
    _update_c_value();
    _update_indicators();
}

void scorekeeper_clear_scores() {
    _clear_score();
}

void scorekeeper_decrease_value(sk_value_ctrl_t vctrl) {
    scorekeeper_add_value(vctrl, -1);
}

void scorekeeper_enable_panel(bool enable) {
    _display_on_panel = enable;
}

void scorekeeper_enable_screen(bool enable) {
    _display_on_screen = enable;
}

int scorekeeper_get_value(sk_value_ctrl_t vctrl) {
    int8_t v;
    switch (vctrl) {
        case SKVALUE_A:
            v = _score_a;
            break;
        case SKVALUE_B:
            v = _score_b;
            break;
        case SKVALUE_C:
            v = _value_c;
            break;
    }
    return v;
}

void scorekeeper_increase_value(sk_value_ctrl_t vctrl) {
    scorekeeper_add_value(vctrl, 1);
}

sk_content_mode_t scorekeeper_mode_get() {
    return _content_mode;
}

void scorekeeper_mode_set(sk_content_mode_t mode) {
    if (_display_on_panel) {
        panel_blank();
    }
    if (_display_on_screen) {
        skscrn_blank();
    }
    _content_mode = mode;
    switch (_content_mode) {
        case SKMODE_SCORES:
            sk_tod_enable(false);
            scorekeeper_update_display();
            break;
        case SKMODE_TOD:
            sk_tod_enable(true);
            break;
        case SKMODE_BLANK:
            // Panel and Screen are blanked above.
            sk_tod_enable(false);
            break;
    }
}

void scorekeeper_set_value(sk_value_ctrl_t vctrl, int v) {
    int8_t *vc;
    switch (vctrl) {
        case SKVALUE_A:
            vc = &_score_a;
            break;
        case SKVALUE_B:
            vc = &_score_b;
            break;
        case SKVALUE_C:
            vc = &_value_c;
            break;
    }
    int8_t value = v;
    switch (_output_mode) {
        case SK_LINEAR_MODE:
            if (value < 0) {
                value = 0;
            }
            if (value > SK_LINEAR_MAX_SCORE) {
                value = SK_LINEAR_MAX_SCORE;
            }
            break;
        case SK_NUMERIC_MODE:
            if (value < 0) {
                value = SK_NUMERIC_MAX_SCORE;
            }
            if (value > SK_NUMERIC_MAX_SCORE) {
                value = 0;
            }
            break;
        default:
            return;
    }
    *vc = value;
    scorekeeper_update_display();
}

void scorekeeper_update_display() {
    _update_a_score();
    _update_b_score();
    _update_c_value();
    _update_indicators();
}

void scorekeeper_module_init() {
    _content_mode = SKMODE_SCORES;
    _output_mode = (panel_type() == PANEL_NUMERIC ? SK_NUMERIC_MODE : SK_LINEAR_MODE);
    _display_on_panel = true;
    _display_on_screen = true;
    sk_screen_module_init(_output_mode);
    sk_tod_module_init();
    scorekeeper_clear_all();

    sk_tod_enable(false);
}
