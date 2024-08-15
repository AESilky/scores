/**
 * Scores main application.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "pico/binary_info.h"
//
#include "system_defs.h" // Main system/board/application definitions
//
#include "board.h"
#include "debug_support.h"
//
#include "be/be.h"
#include "config/config.h"
#include "rc/rc.h"
#include "ui/ui.h"
//
#include "display/oled1106_spi/display_oled1106.h"

#define DOT_MS 60 // Dot at 20 WPM
#define UP_MS  DOT_MS
#define DASH_MS (2 * DOT_MS)
#define CHR_SP (3 * DOT_MS)

 // 'H' (....) 'I' (..)
static const int32_t say_hi[] = {
    DOT_MS,
    UP_MS,
    DOT_MS,
    UP_MS,
    DOT_MS,
    UP_MS,
    DOT_MS,
    CHR_SP,
    DOT_MS,
    UP_MS,
    DOT_MS,
    1000, // Pause before repeating
    0 };

int main()
{
    // useful information for picotool
    bi_decl(bi_program_description("OS and Control for AES Scoreboard"));

    // Board/base level initialization
    if (board_init() != 0) {
        panic("Board init failed.");
    }

    const config_sys_t* system_cfg = config_sys();

    // Indicate that we are awake
    if (debug_mode_enabled()) {
        tone_sound_duration(250);
    }
    int sc = sizeof(char);
    int ss = sizeof(short);
    int si = sizeof(int);
    int sl = sizeof(long);
    debug_printf(true, "Size of char: %d  short: %d  int: %d  long: %d\n", sc, ss, si, sl);
    // Uncomment to force starting in Debug Mode
    //debug_mode_enable(true);

    led_on_off(say_hi);

    sleep_ms(2000);
    //disp_font_test();

    // Set up the Backend (needs to be done before starting the UI)
    be_module_init();

    // Launch the UI (core-1 Message Dispatching Loop)
    start_ui();

    // Enable the IR receivers
    rc_enable_ir(system_cfg->ir1_is_rc, system_cfg->ir2_is_rc);

    // Launch the Backend (core-0 (endless) Message Dispatching Loop
    // (!!! THIS NEVER RETURNS !!!)
    start_be();

    // How did we get here?!
    error_printf(true, "scores - Somehow we are out of our endless message loop in `main()`!!!");
    disp_clear(true);
    disp_string(1, 0, "!!!!!!!!!!!!!!!!", false, true);
    disp_string(2, 0, "! OS LOOP EXIT !", false, true);
    disp_string(3, 0, "!!!!!!!!!!!!!!!!", false, true);
    // ZZZ Reboot!!!
    return 0;
}
