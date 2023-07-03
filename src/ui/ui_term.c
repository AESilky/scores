/**
 * scores User Interface - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_term.h"
#include "config.h"
#include "cmd.h"
#include "cmt.h"
#include "term.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"
#include <ctype.h>
#include <string.h>

static term_color_t _color_term_text_current_bg;
static term_color_t _color_term_text_current_fg;

static ui_term_control_char_handler _control_char_handler[32]; // Room for a handler for each control character

static char _getline_buf[UI_TERM_GETLINE_MAX_LEN_];
static int16_t _getline_index;

static int16_t _output_column;
static char _text_displayed[UI_TERM_COLUMNS+1];

static uint16_t _scroll_end_line;

static ui_term_input_available_handler _input_available_handler;
static ui_term_getline_callback_fn _getline_callback; // Function pointer to be called when an input line is ready

/**
 * @brief Message handler for `MSG_INPUT_CHAR_READY`
 * @ingroup ui
 *
 * @param msg Nothing important in the message.
 */
void _ui_term_handle_input_char_ready(cmt_msg_t* msg) {
    if (NULL != _input_available_handler) {
        _input_available_handler();
    }
}

/**
 * @brief A `term_notify_on_input_fn` handler for input ready.
 * @ingroup ui
 */
void _input_ready_hook(void) {
    // Since this is called by an interrupt handler,
    // post a UI message so that the input is handled
    // by the UI message loop.
    cmt_msg_t msg;
    msg.id = MSG_INPUT_CHAR_READY;
    postUIMsgBlocking(&msg);
    // The hook is cleared on notify, so hook ourself back in.
    term_register_notify_on_input(_input_ready_hook);
}

static void _ui_term_getline_continue() {
    int ci;
    ui_term_getline_callback_fn fn = _getline_callback;

    // Process characters that are available.
    while ((ci = term_getc()) >= 0) {
        char c = (char)ci;
        if ('\n' == c || '\r' == c) {
            // EOL - Terminate the input line and give to callback.
            _getline_buf[_getline_index] = '\0';
            _getline_index = 0;
            _getline_callback = NULL;
            ui_term_register_input_available_handler(NULL);
            fn(_getline_buf);
            return;
        }
        if (BS == c || DEL == c) {
            // Backspace/Delete - move back if we aren't at the BOL
            if (_getline_index > 0) {
                _getline_index--;
                term_cursor_left_1();
                term_erase_char(1);
            }
            _getline_buf[_getline_index] = '\0';
        }
        else if (ESC == c) {
            // Escape - erase the line
            while (_getline_index > 0) {
                _getline_buf[_getline_index] = '\0';
                term_cursor_left_1();
                term_erase_char(1);
                _getline_index--;
            }
            _getline_index = 0;
            _getline_buf[_getline_index] = '\0';
            // If there is a handler registered for ESC let it handle it too
            ui_term_handle_control_character(c);
        }
        else if (c >= ' ' && c < DEL) {
            if (_getline_index < (UI_TERM_GETLINE_MAX_LEN_ - 1)) {
                _getline_buf[_getline_index++] = c;
                putchar(c);
            }
            else {
                // Alert them that they are at the end
                putchar(BEL);
            }
        }
        else {
            // See if there is a handler registered for this, else BEEP
            if (!ui_term_handle_control_character(c)) {
                // Control or 8-bit character we don't deal with
                putchar(BEL);
            }
        }
        // `while` will see if there are more chars available
    }
    // No more input chars are available, but we haven't gotten EOL yet,
    // hook for more to wake back up...
    term_register_notify_on_input(_input_ready_hook);
}

static void _term_init() {
    _input_available_handler = NULL;
    memset(_control_char_handler, 0, sizeof(_control_char_handler));
    memset(_text_displayed, 0, UI_TERM_COLUMNS+1);
    term_reset();
    term_color_default();
    term_set_type(VT_510_TYPE_SPEC, VT_510_ID_SPEC);
    term_set_title(UI_TERM_NAME_VERSION);
    term_set_size(UI_TERM_LINES, UI_TERM_COLUMNS);
    term_clear();
    term_cursor_on(false);
    term_cursor_moveto(1,1);
}

void ui_term_build(void) {
    _term_init();
}

term_color_pair_t ui_term_color_get() {
    term_color_pair_t tc = { _color_term_text_current_fg, _color_term_text_current_bg };
    return (tc);
}

void ui_term_color_refresh() {
    term_color_bg(_color_term_text_current_bg);
    term_color_fg(_color_term_text_current_fg);
}

void ui_term_color_set(term_color_t fg, term_color_t bg) {
    _color_term_text_current_bg = bg;
    _color_term_text_current_fg = fg;
    term_color_bg(bg);
    term_color_fg(fg);
}

static ui_term_control_char_handler ui_term_get_control_char_handler(char c) {
    if (iscntrl(c)) {
        return _control_char_handler[(int)c];
    }
    return (NULL);
}

void ui_term_getline(ui_term_getline_callback_fn getline_cb) {
    _getline_callback = getline_cb;
    ui_term_register_input_available_handler(_ui_term_getline_continue);
    // Use the 'continue' function to process
    _ui_term_getline_continue();
}

void ui_term_getline_cancel(ui_term_input_available_handler input_handler) {
    _getline_callback = NULL;
    ui_term_register_input_available_handler(input_handler);
    _getline_index = 0;
    _getline_buf[_getline_index] = '\0';
}

bool ui_term_handle_control_character(char c) {
    if (iscntrl(c)) {
        ui_term_control_char_handler handler_fn = _control_char_handler[(int)c];
        if (handler_fn) {
            handler_fn(c);
            return (true);
        }
    }
    return (false);
}

static void _printc_for_printf_term(char c, void* arg) {
    putchar(c);
}

int ui_term_printf(const char* format, ...) {
    int pl = 0;
    va_list xArgs;
    va_start(xArgs, format);
    pl += vfctprintf(_printc_for_printf_term, NULL, format, xArgs);
    va_end(xArgs);

    return (pl);
}

static void _putchar_for_code(char c) {
    if ('\n' == c) {
        putchar(c);
        _output_column = 0;
        return;
    }
    if (_output_column == UI_TERM_COLUMNS) {
        // Printing this will cause a wrap.
        if (' ' == c) {
            // It's a space. Just print a newline instead.
            putchar('\n');
            _output_column = 0;
            return;
        }
        else {
            // See if we can move back to a space
            int i = 0;
            for (; i < _output_column; i++) {
                if (' ' == _text_displayed[_output_column - i]) {
                    break;
                }
            }
            if (i < _output_column) {
                // Yes there was a space in the line. Backup, print a '\n', then reprint to the end of line.
                term_cursor_left(i-1);
                term_erase_eol();
                putchar('\n');
                int nc = 0;
                for (int j = ((_output_column - i) + 1); j < _output_column; j++) {
                    putchar(_text_displayed[j]);
                    nc++;
                }
                _output_column = nc;
            }
            else {
                // No spaces in the current line. Just print a '\n' (breaking the word).
                putchar('\n');
                _output_column = 0;
            }
        }
    }
    _text_displayed[_output_column] = c;
    putchar(c);
    _output_column++;
    if ('=' == c) {
        putchar('\n');
        _output_column = 0;
    }
}

void ui_term_puts(char* str) {
    printf("%s", str);
}

void ui_term_register_control_char_handler(char c, ui_term_control_char_handler handler_fn) {
    if (iscntrl(c)) {
        _control_char_handler[(int)c] = handler_fn;
    }
}

void ui_term_register_input_available_handler(ui_term_input_available_handler handler_fn) {
    _input_available_handler = handler_fn;
}

uint16_t ui_term_scroll_end_line_get() {
    return _scroll_end_line;
}

void ui_term_use_cmd_color() {
    ui_term_color_set(UI_TERM_CMD_COLOR_FG, UI_TERM_CMD_COLOR_BG);
}
