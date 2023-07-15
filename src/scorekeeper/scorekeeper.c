/**
 * Scorekeeper functionality.
 *
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "scorekeeper.h"
#include "ui/display/score_panel/panel.h"
#include "ui/segments7/segments7.h"

typedef enum _panel_update_enum {
    NoPU = 0,
    PU
} panel_update_t;

/////////////////////////////////////////////////////////////////////
// Internal function declarations
/////////////////////////////////////////////////////////////////////
//
static void _panel_update_a_score();
static void _panel_update_b_score();


/////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////
//
static uint8_t _score_a;
static uint8_t _score_b;
//static uint8_t _period;

/////////////////////////////////////////////////////////////////////
// Internal functions
/////////////////////////////////////////////////////////////////////
//
static void _clear_score(panel_update_t update) {
    _score_a = 0;
    _score_b = 0;
    if (update == PU) {
        scorekeeper_update_panel_scores();
    }
}

static void _panel_update_a_score() {
    digsegs_t buf[2];
    dig2_int(buf, _score_a);
    panel_A_set(buf);
}

static void _panel_update_b_score() {
    digsegs_t buf[2];
    dig2_int(buf, _score_b);
    panel_B_set(buf);
}


/////////////////////////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////////////////////////
//
void scorekeeper_update_panel_scores() {
    _panel_update_a_score();
    _panel_update_b_score();
}

void scorekeeper_module_init() {
    _clear_score(NoPU);
}
