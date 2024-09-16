/**
 * @brief Application Setup functionality.
 * @ingroup setup
 *
 * This provides application level setup via the remote control and/or the switch panels.
 * It uses the oled display panel to interact with the user.
 *
 * Copyright 2024 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _SETUP_H_
#define _SETUP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "curswitch/curswitch_t.h"
#include "rc/rc_t.h"

#include <stdbool.h>

/**
 * @brief Function prototype for a Setup App callback function.
 * @ingroup setup
 */
typedef void (*setup_callback_fn)(void);

/**
 * @brief Run the Setup app.
 * @ingroup setup
 *
 * Runs the application to interact with the user on the oled display
 * using either the remote or the switch bank(s).
 *
 * @param cb Setup callback function that is called when the app exits
 * @return true if the app started, false if it was already running.
 */
extern bool setup_app_run(setup_callback_fn cb);

extern void setup_app_rc_action(rc_action_data_t action, bool longpress);

extern void setup_app_rc_entry(rc_value_entry_t entry);

/**
 * @brief To be called by the switch banks handler to let the Setup app get
 *      input from the user.
 * @ingroup setup
 *
 * @param bank The bank of the switch pressed or released.
 * @param sw_id The switch pressed or released.
 * @param pressed True if the switch was pressed, false if released.
 * @param long_press True if this is a long-press.
 * @param repeat True if this is a repeat.
 */
extern void setup_app_switch_action(switch_bank_t bank, switch_id_t sw_id, bool pressed, bool long_press, bool repeat);

extern void setup_module_init(void);

#ifdef __cplusplus
    }
#endif
#endif // _SETUP_H_
