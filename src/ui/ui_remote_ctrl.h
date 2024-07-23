/**
 * Remote control handling for Scorekeeper.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SK_RC_H_
#define _SK_RC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "rc/rc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ==========================================================================
// Handlers for the CTRL processing.
// ==========================================================================

/**
 * @brief Decrease the value of one of the scores or the period/time.
 *
 * What to decrease will be based on the code.
 *
 * @param code One of the 'Decrease' buttons.
 * @param repeat It is possible for the repeat to be on.
 */
extern void ui_rc_decrease_value(uint8_t code, bool repeat);

/**
 * @brief Handler for Remote Control Button 'B' (035) to blank display.
 */
extern void ui_rc_display_blank(uint8_t code, bool repeat);

/**
 * @brief Handler for Remote Control Button 'D' (037) to enable score display.
 */
extern void ui_rc_display_scores(uint8_t code, bool repeat);

/**
 * @brief Handler for Remote Control Button 'C' (36) to enable Time-of-Day
 * display on the panel.
 */
extern void ui_rc_display_tod(uint8_t code, bool repeat);

/**
 * @brief Increase the value of one of the scores or the period/time.
 *
 * What to increase will be based on the code.
 *
 * @param code One of the 'Increase' buttons.
 * @param repeat It is possible for the repeat to be on.
 */
extern void ui_rc_increase_value(uint8_t code, bool repeat);

/**
 * @brief Register the handlers with the Remote Control handling.
 *
 * This will register all of the scorekeeper remote control handlers. It can
 * be called at any time to register/re-register the handlers.
 */
extern void ui_rc_register_handlers(void);

#ifdef __cplusplus
}
#endif
#endif // _SK_RC_H_
