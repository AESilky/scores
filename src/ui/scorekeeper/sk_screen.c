/**
 * Scorekeeper display on the OLED screen.
 *
 *
 * Copyright 2024 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "sk_screen.h"
#include "display/oled1106_spi/display_oled1106.h"

#include <stdio.h>

/////////////////////////////////////////////////////////////////////
// Data
/////////////////////////////////////////////////////////////////////
//
#define A_COL     0
#define A_ROW     1
#define B_COL    12
#define B_ROW     1
#define PT_COL    6
#define PT_ROW    3
#define INDA_COL  0
#define INDA_ROW  2
#define INDB_COL 10
#define INDB_ROW  2

static sk_output_mode_t _output_mode;

/////////////////////////////////////////////////////////////////////
// Internal function declarations
/////////////////////////////////////////////////////////////////////
//
void _get_two_chars(char buf[3], const char *from);

/////////////////////////////////////////////////////////////////////
// Internal functions
/////////////////////////////////////////////////////////////////////
//
void _display_2chars_at(uint16_t row, uint16_t col, const char *value) {
    char buf[3];
    _get_two_chars(buf, value);
    disp_string(row, col, buf, true, true);
}

void _display_ind_at(uint16_t row, uint16_t col, uint8_t n) {
    char buf[5];
    uint8_t mask = 0x08;
    for (int i = 0; i < 4; i++) {
        char v = (n & mask) ? 0x0B : 0x0A; // Checked Radio | Unchecked Radio
        buf[i] = v;
        mask = mask >> 1;
    }
    buf[4] = 0;
    disp_string(row, col, buf, false, true);
}

void _get_two_chars(char buf[3], const char *from) {
    for (int i=0; i<2; i++) {
        char c = *(from+i);
        buf[i] = c;
        if (c == 0) {
            break;
        }
    }
    buf[2] = 0;
}

/////////////////////////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////////////////////////
//
void skscrn_blank() {
    disp_clear(true);
    char* str = (_output_mode == SK_LINEAR_MODE ? "L" : "N");
    disp_string(5, 0, str, false, true);
}

void skscrn_A_set(uint8_t value) {
    char buf[3];
    sprintf(buf, "%02d", value);
    skscrn_A_set_str(buf);
}

void skscrn_A_set_str(const char *value){
    _display_2chars_at(A_ROW, A_COL, value);
}

void skscrn_B_set(uint8_t value) {
    char buf[3];
    sprintf(buf, "%02d", value);
    skscrn_B_set_str(buf);
}

void skscrn_B_set_str(const char *value){
    _display_2chars_at(B_ROW, B_COL, value);
}

void skscrn_PT_set(uint8_t value) {
    char buf[3];
    sprintf(buf, "%02d", value);
    skscrn_PT_set_str(buf);
}

void skscrn_PT_set_str(const char *value){
    _display_2chars_at(PT_ROW, PT_COL, value);
}

void skscrn_IND_set(uint8_t value){
    uint8_t n = (value >> 4);
    skscrn_INDA_set(n);
    n = (value & 0x0F);
    skscrn_INDB_set(n);
}

void skscrn_INDA_set(uint8_t value){
    _display_ind_at(INDA_ROW, INDA_COL, value);
}

void skscrn_INDB_set(uint8_t value){
    _display_ind_at(INDB_ROW, INDB_COL, value);
}

///

void sk_screen_module_init(sk_output_mode_t output_mode) {
    _output_mode = output_mode;
    skscrn_blank();
}
