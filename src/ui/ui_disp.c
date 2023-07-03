/**
 * scores User Interface - On the display, rotary, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_disp.h"
#include "config.h"
#include "display_oled1306.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"

#include <stdlib.h>
#include <string.h>

static void _header_fill_fixed() {
}

static void _status_fill_fixed() {
}

void ui_disp_build(void) {
}

void ui_disp_puts(char* str) {
    disp_string(0, 0, str, false, true); // ZZZ need to implement cursor-based functions
}

