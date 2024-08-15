/**
 * Scorekeeper base functionality.
 *
 *
 * Copyright 2023-24 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SCOREKEEPER_H_
#define _SCOREKEEPER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ui/cmd/cmd_t.h" // Command processing type definitions

/**
 * @brief Mode for the content shown on the panel and screen.
 */
typedef enum _sk_content_mode_ {
    SKMODE_SCORES = 1,
    SKMODE_TOD,
    SKMODE_BLANK,
} sk_content_mode_t;

typedef enum _sk_output_mode_ {
    SK_LINEAR_MODE      = 1,
    SK_NUMERIC_MODE     = 2, 
} sk_output_mode_t;

#define SK_LINEAR_MAX_SCORE     24
#define SK_NUMERIC_MAX_SCORE    99

typedef enum _sk_value_ctrl_ {
    SKVALUE_A = 1,
    SKVALUE_B = 2,
    SKVALUE_C = 3,
} sk_value_ctrl_t;

extern const cmd_handler_entry_t cmd_ui_remote_code_entry;


/**
 * @brief Add a value to the current value for the given control set.
 *
 * If the result exceeds the limit, it will be capped to the limit.
 *
 * @param vctrl sk_value_ctrl_t to add the value to.
 * @param v int value to add (can be negative)
 */
extern void scorekeeper_add_value(sk_value_ctrl_t vctrl, int v);

/**
 * @brief Clear all scorekeeper values, A Score, B Score, Period/Time, Indicators.
 * @ingroup scorekeeper
 */
extern void scorekeeper_clear_all();

/**
 * @brief Clear the scores.
 * @ingroup scorekeeper
 *
 * Clear (zero out) the A and B scores.
 */
extern void scorekeeper_clear_scores();

/**
 * @brief Decrease the value for the given control set.
 *
 * The value will decrease by an amount based on the current value mode.
 *
 * @param vctrl sk_value_ctrl_t to decrease the value of.
 */
extern void scorekeeper_decrease_value(sk_value_ctrl_t vctrl);

/**
 * @brief Enable/Disable score display on the panel.
 *
 * @param enable True to enable display of the scores on the panel.
 */
extern void scorekeeper_enable_panel(bool enable);

/**
 * @brief Enable/Disable score display on the display screen.
 *
 * @param enable True to enable display of the scores on the screen.
 */
extern void scorekeeper_enable_screen(bool enable);

/**
 * @brief Get the value for the given control set.
 * 
 * @param vctrl sk_value_ctrl_t to decrease the value of.
 * @return int value of the control set
 */
extern int scorekeeper_get_value(sk_value_ctrl_t vctrl);

/**
 * @brief Increase the value for the given control set.
 *
 * The value will increase by an amount based on the current value mode.
 *
 * @param vctrl sk_value_ctrl_t to decrease the value of.
 */
extern void scorekeeper_increase_value(sk_value_ctrl_t vctrl);

/**
 * @brief Get the current mode of the scorekeeper display.
 *
 * @return sk_content_mode_t The current mode.
 */
extern sk_content_mode_t scorekeeper_mode_get();

/**
 * @brief Set the mode of the scorekeeper display.
 *
 * @param mode sk_content_mode_t Mode
 */
extern void scorekeeper_mode_set(sk_content_mode_t mode);

/**
 * @brief Set a specific value for the given control set.
 * 
 * The value will be limited to 0 -> v -> MAX
 * 
 * @param vctrl sk_value_ctrl_t to decrease the value of.
 * @param v int value to set
 */
extern void scorekeeper_set_value(sk_value_ctrl_t vctrl, int v);

/**
 * @brief Update the score panel/screen with the current scores, period/time, and
 *          indicators.
 * @ingroup scorekeeper
 */
extern void scorekeeper_update_display();

/**
 * @brief Initialize the Scorekeeper.
 * @ingroup scorekeeper
 */
extern void scorekeeper_module_init();

#ifdef __cplusplus
}
#endif
#endif // _SCOREKEEPER_H_
