/**
 * Scorekeeper application functionality.
 *
 *
 * Copyright 2023-24 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SK_APP_H_
#define _SK_APP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "curswitch/curswitch_t.h"
#include "rc/rc_t.h"

extern void sk_app_rc_action(rc_action_data_t action);

extern void sk_app_rc_entry(rc_value_entry_t entry);

extern void sk_app_switch_action(switch_bank_t bank, switch_id_t sw_id, bool pressed, bool long_press, bool repeat);

extern void sk_app_module_init();

#ifdef __cplusplus
}
#endif
#endif // _SK_APP_H_

