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
#include "rc.h"

#include "ui/ui_term.h"

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include <stdlib.h>
#include <string.h>

// /////////////// Data ////////////////
remote_code_handler_fn _handlers[CTRL_CODES_NUM];

static void _code_zero_handler(uint8_t code, bool repeat);
static void _code_unused_handler(uint8_t code, bool repeat);

void _code_unused_handler(uint8_t code, bool repeat) {
    ui_term_printf("Remote: %0.3d Repeat: %0.1d (unused)\n", code, repeat);
}

void _code_zero_handler(uint8_t code, bool repeat) {
    ui_term_printf("Remote: 000\n");
}

void _handle_code(int16_t code) {
    bool repeat = (code < 0); // Negative values indicate a repeated code
    uint8_t c = abs(code) % CTRL_CODE_MAX;
    remote_code_handler_fn handler = _handlers[c];
    if (0 == c) {
        _code_zero_handler(c, repeat);
    }
    else if (handler) {
        handler(c, repeat);
    }
    else {
        _code_unused_handler(c, repeat);
    }
}

// Public functions

void rc_handle_code(int16_t code) {
    _handle_code(code);
}

remote_code_handler_fn rc_register_handler(uint8_t code, remote_code_handler_fn handler) {
    uint8_t c = abs(code) % CTRL_CODE_MAX;
    remote_code_handler_fn existing = _handlers[c];
    _handlers[c] = handler;

    return (existing);
}

// //////////// Module Init /////////////
void rc_module_init(void) {
    memset(_handlers, 0, CTRL_CODES_NUM * sizeof(remote_code_handler_fn));
}