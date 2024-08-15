/**
 * @brief Remote Control types.
 * @ingroup rc
 *
 * This provides type definitions for command processing from a IR/Radio Remote.
 *
 * Copyright 2024 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _RC_T_H_
#define _RC_T_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum _rc_ir_source_ {
    IR_NONE = 0,
    IR_A = 1,
    IR_B = 2
} rc_ir_source_t;

/**
 * @brief Virtual Remote (button) codes
 * @ingroup rc
 */
typedef enum _rc_vcode_ {
    RC_NULL            =  0,   // NULL value
    RC_INPUT           =  1,   //
    RC_POWER           =  2,   //
    RC_MUTE            =  3,   //
    RC_VOL_UP          =  4,   //
    RC_VOL_DOWN        =  5,   //
    RC_CH_UP           =  6,   //
    RC_CH_DOWN         =  7,   //
    RC_BACK            =  8,   //
    RC_INFO            =  9,   //
    RC_NUM_0           = 10,   //
    RC_NUM_1           = 11,   //
    RC_NUM_2           = 12,   //
    RC_NUM_3           = 13,   //
    RC_NUM_4           = 14,   //
    RC_NUM_5           = 15,   //
    RC_NUM_6           = 16,   //
    RC_NUM_7           = 17,   //
    RC_NUM_8           = 18,   //
    RC_NUM_9           = 19,   //
    RC_MOVE_BACK       = 20,   // Row of 3a - 1
    RC_PLAY_PAUSE      = 21,   // Row of 3a - 2
    RC_MOVE_FORWARD    = 22,   // Row of 3a - 3
    RC_EXIT            = 23,   // Row of 3b - 1
    RC_DOT             = 24,   // Row of 3b - 2
    RC_PAGE_UP         = 25,   // Row of 3b - 3
    RC_GUIDE           = 26,   // Row of 3c - 1
    RC_LOGO            = 27,   // Row of 3c - 2
    RC_PAGE_DOWN       = 28,   // Row of 3c - 3
    RC_OK              = 29,   // Cursor Pad - OK (center)
    RC_CURSOR_UP       = 30,   // Cursor Pad - UP
    RC_CURSOR_RIGHT    = 31,   // Cursor Pad - RIGHT
    RC_CURSOR_DOWN     = 32,   // Cursor Pad - DOWN
    RC_CURSOR_LEFT     = 33,   // Cursor Pad - LEFT
    RC_A               = 34,   // Letter Buttons
    RC_B               = 35,   // Letter Buttons
    RC_C               = 36,   // Letter Buttons
    RC_D               = 37,   // Letter Buttons
    RC_MIC             = 38,   // Microphone/Talk
    RC_MENU            = 39,
    RC_MENU_3BAR       = 40,   // The 'Hamburger'
    RC_HOME            = 41,
    RC_ENTER           = 42,
    RC_MINUS           = 43,
} rc_vcode_t;

typedef struct _rc_ir_frame_ {
    uint16_t data;
    uint16_t addr;
    rc_ir_source_t src;
    bool repeat;
    uint32_t ts_ms;
} rc_ir_frame_t;

typedef struct _rc_action_ {
    rc_vcode_t code;
    bool repeat;
    uint32_t ts_ms;
} rc_action_data_t;

typedef struct _rc_value_entry_ {
    rc_vcode_t code;
    int value;
    int divisor;
} rc_value_entry_t;

#ifdef __cplusplus
    }
#endif
#endif // _RC_T_H_

