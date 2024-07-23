/**
 * Scores Back-End - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _BE_H_
#define _BE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt/cmt.h"

#define BE_CORE_NUM 0

#define SWITCH_LONGPRESS_DEFAULT 800 // 0.8 seconds (800ms) is considered a 'long press'
#define SWITCH_REPEAT_MS 250 // If a switch is long-pressed, repeat it every 1/4 second.

/**
 * @brief Message loop context for use by the loop handler.
 * @ingroup backend
 */
extern msg_loop_cntx_t be_msg_loop_cntx;

/**
 * @brief Initialize the back-end
 * @ingroup backend
 */
extern void be_module_init(void);

/**
 * @brief Start the Backend (core 0 (endless) message-loop).
 * @ingroup backend
 */
extern void start_be(void);


#ifdef __cplusplus
}
#endif
#endif // _BE_H_
