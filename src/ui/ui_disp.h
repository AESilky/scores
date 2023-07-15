/**
 * scores User Interface - On the display, rotary, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _UI_DISP_H_
#define _UI_DISP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt/cmt.h"

#include "pico/types.h"

#include <stdint.h>

/**
 * @brief Build (or rebuild) the UI on the display.
 * @ingroup ui
 */
extern void ui_disp_build(void);

/**
 * @brief Print a string in the scrolling (code) area of the screen.
 * @ingroup ui
 *
 * If code is displaying, this will print a newline and then the string.
 *
 * @param str The string to print.
 */
void ui_disp_puts(char* str);

#ifdef __cplusplus
}
#endif
#endif // _UI_DISP_H_