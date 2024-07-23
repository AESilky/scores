/**
 * scores User Interface - On the display, rotary, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_disp.h"

#include "config/config.h"
#include "display/oled1106_spi/display_oled1106.h"
#include "util/util.h"

#include "hardware/rtc.h"

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

