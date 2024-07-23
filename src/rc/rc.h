/**
 * @brief Remote Control functionality.
 * @ingroup rc
 *
 * This provides command processing from a IR/Radio Remote. It supports 255 codes received from
 * the remote device. It calls a handler function for the received code.
 *
 * Copyright 2024 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _CTRL_H_
#define _CTRL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cmt/cmt.h"

// //////////////// Virtual Remote Code to Logical (X15 Remote) ///////////////
#define RC_TV_INPUT         1   //
#define RC_POWER            2   //
#define RC_MUTE             3   //
#define RC_VOL_UP           4   //
#define RC_VOL_DOWN         5   //
#define RC_CH_UP            6   //
#define RC_CH_DOWN          7   //
#define RC_BACK             8   //
#define RC_INFO             9   //
#define RC_NUM_0           10  //
#define RC_NUM_1           11   //
#define RC_NUM_2           12   //
#define RC_NUM_3           13   //
#define RC_NUM_4           14   //
#define RC_NUM_5           15   //
#define RC_NUM_6           16   //
#define RC_NUM_7           17   //
#define RC_NUM_8           18   //
#define RC_NUM_9           19   //
#define RC_MOVE_BACK       20   // Row of 3a - 1
#define RC_PLAY_PAUSE      21   // Row of 3a - 2
#define RC_MOVE_FORWARD    22   // Row of 3a - 3
#define RC_EXIT            23   // Row of 3b - 1
#define RC_DOT             24   // Row of 3b - 2
#define RC_PAGE_UP         25   // Row of 3b - 3
#define RC_GUIDE           26   // Row of 3c - 1
#define RC_LOGO            27   // Row of 3c - 2
#define RC_PAGE_DOWN       28   // Row of 3c - 3
#define RC_OK              29   // Cursor Pad - OK (center)
#define RC_CURSOR_UP       30   // Cursor Pad - UP
#define RC_CURSOR_RIGHT    31   // Cursor Pad - RIGHT
#define RC_CURSOR_DOWN     32   // Cursor Pad - DOWN
#define RC_CURSOR_LEFT     33   // Cursor Pad - LEFT
#define RC_A               34   // Letter Buttons around Cursor Pad
#define RC_B               35   // Letter Buttons around Cursor Pad
#define RC_C               36   // Letter Buttons around Cursor Pad
#define RC_D               37   // Letter Buttons around Cursor Pad
#define RC_MIC             38   // Microphone/Talk

/**
 * @brief The maximum code value.
 */
#define CTRL_CODE_MAX 63

/**
 * @brief The number of Control Codes handled.
 */
#define CTRL_CODES_NUM 64

/**
 * @brief Function prototype for the remote code handler.
 * @ingroup rc
 *
 * @param code - The code. Negative values indicate a repeat.
 */
typedef void (*remote_code_handler_fn)(uint8_t code, bool repeat);

/**
 * @brief Handle a remote code.
 *
 * @param code A remote code from -63 to 0 to 63. Negative values indicate 'repeat' of the (positive) code.
 */
extern void rc_handle_code(int16_t code);

/**
 * @brief Register a handler for a control code.
 *
 * @param code The code to register the handler for.
 * @param handler The handler function to be called for the code
 * @return remote_code_handler_fn The handler that was registered for the code (or null)
 */
extern remote_code_handler_fn rc_register_handler(uint8_t code, remote_code_handler_fn handler);

/**
 * @brief Initialize the command processor.
 * @ingroup ui
 */
extern void rc_module_init(void);


#ifdef __cplusplus
    }
#endif
#endif // _CTRL_H_


