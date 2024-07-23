/**
 * Time of day display on the panel.
 *
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SK_TOD_H_
#define _SK_TOD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt/cmt.h"

/**
 * @brief UI Message Loop handler to update the time of day on the panel.
 * @ingroup sk_tod
 *
 * @param msg The received message
 */
extern void _sk_tod_update_handler(cmt_msg_t* msg);
extern const msg_handler_entry_t _sk_tod_update_handler_entry;

/**
 * @brief Turn time display on/off.
 * @ingroup sk_tod
 *
 * Enable or disable display of the time of day.
 */
extern void sk_tod_enable(bool enable);

extern void sk_tod_module_init();

#ifdef __cplusplus
}
#endif
#endif // _SK_TOD_H_
