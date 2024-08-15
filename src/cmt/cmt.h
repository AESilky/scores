/**
 * scores Cooperative Multi-Tasking.
 *
 * Containes message loop, timer, and other CMT enablement functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _MK_CMT_H_
#define _MK_CMT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "curswitch/curswitch_t.h"
#include "gfx/gfx.h"
#include "rc/rc_t.h"

#include "pico/types.h"

#define SCHEDULED_MESSAGES_MAX 16

typedef enum _MSG_ID_ {
    // Common messages (used by both BE and UI)
    MSG_COMMON_NOOP = 0x0000,
    MSG_CONFIG_CHANGED,
    MSG_CMT_SLEEP,
    MSG_DEBUG_CHANGED,
    MSG_INPUT_SW_PRESS,
    MSG_INPUT_SW_RELEASE,
    MSG_RC_ACTION,
    MSG_RC_LONGPRESS,
    MSG_RC_VALUE_ENTERED,
    MSG_PANEL_BLINK_FAST_TGL,
    MSG_PANEL_BLINK_SLOW_TGL,
    MSG_PANEL_REPEAT_21MS,
    MSG_SWITCH_ACTION,
    MSG_SWITCH_LONGPRESS,
    //
    // Back-End messages
    MSG_BACKEND_NOOP = 0x0100,
    MSG_BE_TEST,
    MSG_INPUT_SW_DEBOUNCE,
    MSG_IR_FRAME_RCVD,
    MSG_STDIO_CHAR_READY,
    MSG_B1SW_LONGPRESS_DELAY,
    MSG_B2SW_LONGPRESS_DELAY,
    MSG_UI_INITIALIZED,
    //
    // Front-End/UI messages
    MSG_UI_NOOP = 0x0200,
    MSG_BE_INITIALIZED,
    MSG_CMD_INIT_TERMINAL,
    MSG_DISPLAY_MESSAGE,
    MSG_PANEL_TOD_UPDATE,
    MSG_SHELL_START,
    MSG_WIFI_CONN_STATUS_UPDATE,
} msg_id_t;

/**
 * @brief Function prototype for a sleep function.
 * @ingroup cmt
 */
typedef void (*cmt_sleep_fn)(void* user_data);

typedef struct _cmt_sleep_data_ {
    cmt_sleep_fn sleep_fn;
    void* user_data;
} cmt_sleep_data_t;

/**
 * @brief Message data.
 *
 * Union that can hold the data needed by the messages.
 */
typedef union _MSG_DATA_VALUE {
    char c;
    bool bv;
    bool debug;
    cmt_sleep_data_t cmt_sleep;
    rc_ir_frame_t ir_frame;
    rc_action_data_t rc_action;
    rc_value_entry_t rc_entry;
    int32_t status;
    char* str;
    switch_action_data_t sw_action;
    uint32_t ts_ms;
    uint64_t ts_us;
} msg_data_value_t;

/**
 * @brief Structure containing a message ID and message data.
 *
 * @param id The ID (number) of the message.
 * @param data The data for the message.
 * @param t The millisecond time msg was posted (set by the posting system)
 */
typedef struct _CMT_MSG {
    msg_id_t id;
    msg_data_value_t data;
    uint32_t t;
} cmt_msg_t;

/**
 * @brief Handler Entry for the CMT Sleep. This is put in both message loop
 *      handler lists, so a sleep can be handled for either.
 * @ingroup cmt
 *
 */
extern const msg_handler_entry_t cmt_sm_tick_handler_entry;


#include "multicore.h"

// Define functional names for the 'Core' message queue functions (Camel-case to help flag as aliases).
#define getBEMsgBlocking( pmsg )        get_core1_msg_blocking( pmsg )
#define getBEMsgNoWait( pmsg )          get_core1_msg_nowait( pmsg )
#define getUIMsgBlocking( pmsg )        get_core0_msg_blocking( pmsg )
#define getUIMsgNoWait( pmsg )          get_core0_msg_nowait( pmsg )
#define postBEMsgBlocking( pmsg )       post_to_core0_blocking( pmsg )
#define postBEMsgNoWait( pmsg )         post_to_core0_nowait( pmsg )
#define postUIMsgBlocking( pmsg )       post_to_core1_blocking( pmsg )
#define postUIMsgNoWait( pmsg )         post_to_core1_nowait( pmsg )
#define postBothMsgBlocking( pmsg )     post_to_cores_blocking( pmsg )
#define postBothMsgNoWait( pmsg )       post_to_cores_nowait( pmsg )

/**
 * @brief Function prototype for an idle function.
 * @ingroup cmt
 */
typedef void (*idle_fn)(void);

/**
 * @brief Function prototype for a message handler.
 * @ingroup cmt
 *
 * @param msg The message to handle.
 */
typedef void (*msg_handler_fn)(cmt_msg_t* msg);


typedef struct _MSG_HANDLER_ENTRY {
    int msg_id;
    msg_handler_fn msg_handler;
} msg_handler_entry_t;

typedef struct _PROC_STATUS_ACCUM_ {
    volatile int64_t cs;
    volatile uint64_t ts_psa;                       // Timestamp of last PS Accumulator/sec update
    volatile uint64_t t_active;
    volatile uint64_t t_idle;
    volatile uint64_t t_msg_retrieve;
    volatile uint32_t retrieved;
    volatile uint32_t idle;
    volatile uint32_t interrupt_status;
    volatile float core_temp;
} proc_status_accum_t;

typedef struct _MSG_LOOP_CNTX {
    uint8_t corenum;                                // The core number the loop is running on
    const msg_handler_entry_t** handler_entries;    // NULL terminated list of message handler entries
    const idle_fn* idle_functions;                  // Null terminated list of idle functions
} msg_loop_cntx_t;

/**
 * @brief Indicates if the Core-0 message loop has been started.
 * @ingroup cmt
 *
 * @return true The Core-0 message loop has been started.
 * @return false The Core-0 message loop has not been started yet.
 */
extern bool cmt_message_loop_0_running();

/**
 * @brief Indicates if the Core-1 message loop has been started.
 * @ingroup cmt
 *
 * @return true The Core-1 message loop has been started.
 * @return false The Core-1 message loop has not been started yet.
 */
extern bool cmt_message_loop_1_running();

/**
 * @brief Indicates if both the Core-0 and Core-1 message loops have been started.
 * @ingroup cmt
 *
 * @return true The message loops have been started.
 * @return false The message loops have not been started yet.
 */
extern bool cmt_message_loops_running();

/**
 * @brief Handle a Scheduled Message timer Tick.
 *
 * @param msg (not used)
 */
extern void cmt_handle_sleep(cmt_msg_t* msg);

/**
 * @brief Get the last Process Status Accumulator per second values.
 *
 * @param psas Pointer to Process Status Accumulator structure to fill with values.
 * @param corenum The core number (0|1) to get the process status values for.
 */
extern void cmt_proc_status_sec(proc_status_accum_t* psas, uint8_t corenum);

/**
 * @brief The number of scheduled messages waiting.
 *
 * @return int Number of scheduled messages.
 */
extern int cmt_sched_msg_waiting();

/**
 * @brief Get the ID's of the scheduled messages waiting.
 *
 * @param max The maximum number of ID's to return
 * @param buf Buffer (of uint16's) to hold the values
 *
 * @return True is any messages are waiting
 */
extern bool cmt_sched_msg_waiting_ids(int max, uint16_t *buf);

/**
 * @brief Sleep for milliseconds and call a function.
 * @ingroup cmt
 *
 * @param ms The time in milliseconds from now.
 * @param sleep_fn The function to call when the time expires.
 * @param user_data A pointer to user data that the 'sleep_fn' will be called with.
 */
extern void cmt_sleep_ms(int32_t ms, cmt_sleep_fn sleep_fn, void* user_data);

/**
 * @brief Schedule a message to post to Core-0 in the future.
 *
 * Use this when it is needed to future post to a core other than the one currently
 * being run on.
 *
 * @param ms The time in milliseconds from now.
 * @param msg The cmt_msg_t message to post when the time period elapses.
 */
extern void schedule_core0_msg_in_ms(int32_t ms, const cmt_msg_t* msg);

/**
 * @brief Schedule a message to post to Core-1 in the future.
 *
 * Use this when it is needed to future post to a core other than the one currently
 * being run on.
 *
 * @param ms The time in milliseconds from now.
 * @param msg The cmt_msg_t message to post when the time period elapses.
 */
extern void schedule_core1_msg_in_ms(int32_t ms, const cmt_msg_t* msg);

/**
 * @brief Schedule a message to post in the future.
 *
 * @param ms The time in milliseconds from now.
 * @param msg The cmt_msg_t message to post when the time period elapses.
 */
extern void schedule_msg_in_ms(int32_t ms, const cmt_msg_t* msg);

/**
 * @brief Cancel scheduled message(s) for a message ID.
 * @ingroup cmt
 *
 * This will attempt to cancel the scheduled message. It is possible that the time might have already
 * passed and the message was posted.
 *
 * @param sched_msg_id The ID of the message that was scheduled.
 */
extern void scheduled_msg_cancel(msg_id_t sched_msg_id);

/**
 * @brief Indicate if a scheduled message exists.
 * @ingroup cmt
 *
 * Typically, this is used to keep from adding a scheduled message if one already exists.
 *
 * @param sched_msg_id The ID of the message to check for.
 * @return True if there is a scheduled message for the ID.
 */
extern bool scheduled_message_exists(msg_id_t sched_msg_id);

/**
 * @brief Enter into a message processing loop.
 * @ingroup cmt
 *
 * Enter into a message processing loop using a loop context.
 * This function will not return.
 *
 * @param loop_context Loop context for processing.
 */
extern void message_loop(const msg_loop_cntx_t* loop_context);

/**
 * @brief Initialize the Cooperative Multi-Tasking system.
 * @ingroup cmt
 */
extern void cmt_module_init();

#ifdef __cplusplus
    }
#endif
#endif // _MK_CMT_H_
