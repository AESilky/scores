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
#ifndef _RCTRL_H_
#define _RCTRL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "rc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cmt/cmt.h"


/**
 * @brief The maximum code value.
 * @ingroup rc
 */
#define CTRL_CODE_MAX 63

/**
 * @brief The number of Control Codes handled.
 * @ingroup rc
 */
#define CTRL_CODES_NUM 64

/**
 * @brief Message handler entry for IR frames received.
 * @ingroup rc
 *
 * IR frames are the (nearly) raw information detected.
 */
extern const msg_handler_entry_t _os_ir_frame_handler_entry;

/**
 * @brief Message handler entry for Remote Control actions.
 * @ingroup rc
 *
 * Remote Control (RC) actions are button presses.
 */
extern const msg_handler_entry_t _os_rc_action_handler_entry;

/**
 * @brief Function prototype for the remote code handler.
 * @ingroup rc
 *
 * @param code - The code. Negative values indicate a repeat.
 */
typedef void (*remote_code_handler_fn)(uint8_t code, bool repeat);

/**
 * @brief Enable the IR PIO state machines and the IRQ.
 * @ingroup rc
 *
 * The PIO state machines are configured in the module init, but the state machines and irq are not
 * started/enabled, so that interrupts won't be generated until everything is ready for them. This
 * starts the state machines for the enabled IR ports and if either is enabled it enables the interrupt.
 */
extern void rc_enable_ir(bool ir_a_enabled, bool ir_b_enabled);

/**
 * @brief Handle a remote code.
 * @ingroup rc
 *
 * @param code A remote code from -63 to 0 to 63. Negative values indicate 'repeat' of the (positive) code.
 */
extern void rc_handle_code(int16_t code);

/**
 * @brief Indicate if a value is in the process of being collected.
 * @ingroup rc
 *
 * @return true A value is being collected
 * @return false Not collecting a value
 */
extern bool rc_is_collecting_value();

/**
 * @brief Clear the collecting of a value. Set the collected value to 0.
 * @ingroup rc
 *
 * After a value has been collected, this must be called in order to collect again.
 */
extern void rc_value_collecting_reset();

/**
 * @brief Initialize the command processor.
 * @ingroup rc
 *
 * @param ir_a_enabled True if the Front IR (A) should be enabled
 * @param ir_b_enabled True if the Rear IR (B) should be enabled
 */
extern void rc_module_init(bool ir_a_enabled, bool ir_b_enabled);


#ifdef __cplusplus
    }
#endif
#endif // _RCTRL_H_


