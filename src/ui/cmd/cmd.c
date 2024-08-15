/**
 * scores CMD Command shell - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */

#include "cmd.h"

#include "system_defs.h"

#include "debug_support.h"
#include "cmt/cmt.h"
#include "config/config.h"
#include "config/config_cmd.h"
#include "ui/scorekeeper/scorekeeper.h"
#include "ui/ui_term.h"
#include "term/term.h"
#include "util/util.h"

#include <string.h>

#define CMD_LINE_MAX_ARGS 64

// Buffer to copy the input line into to be parsed.
static char _cmdline_parsed[UI_TERM_GETLINE_MAX_LEN_];

// Command processor declarations
static int _cmd_help(int argc, char** argv, const char* unparsed);
static int _cmd_keys(int argc, char** argv, const char* unparsed);
static int _cmd_proc_status(int argc, char** argv, const char* unparsed);

// Command processors framework
static const cmd_handler_entry_t _cmd_help_entry = {
    _cmd_help,
    1,
    "help",
    "[-a|--all] [command_name [command_name...]]",
    "List of commands or information for a specific command(s).\n  -a|--all : Display hidden commands.\n",
};
static const cmd_handler_entry_t _cmd_keys_entry = {
    _cmd_keys,
    4,
    "keys",
    "",
    "List of the keyboard control key actions.\n",
};
static const cmd_handler_entry_t _cmd_proc_status_entry = {
    _cmd_proc_status,
    3,
    ".ps",
    "[-m|--msg]",
    "Display process status per second.\n  -m|--msg : Display MSG ID of scheduled messages.\n",
};

/**
 * @brief List of Command Handlers
 */
static const cmd_handler_entry_t* _command_entries[] = {
    & cmd_debug_support_entry,  // .debug - 'DOT' commands come first
    & _cmd_proc_status_entry,   // .ps
    & cmd_bootcfg_entry,
    & cmd_cfg_entry,
    & cmd_configure_entry,
    & _cmd_help_entry,
    & _cmd_keys_entry,
    & cmd_load_entry,
    & cmd_save_entry,
    & cmd_ui_remote_code_entry,
    ((cmd_handler_entry_t*)0),  // Last entry must be a NULL
};


// Internal (non-command) declarations

static void _hook_keypress();

// Class data

static cmd_state_t _cmd_state = CMD_COLLECTING_LINE;


// Command functions

static int _cmd_help(int argc, char** argv, const char* unparsed) {
    const cmd_handler_entry_t** cmds;
    const cmd_handler_entry_t* cmd;
    bool disp_commands = true;
    bool disp_hidden = false;

    argv++;
    if (argc > 1) {
        // They entered an option and/or command names
        if (strcmp("-a", *argv) == 0 || strcmp("--all", *argv) == 0) {
            disp_hidden = true;
            argv++; argc--;
        }
    }
    if (argc > 1) {
        char* user_cmd;
        for (int i = 1; i < argc; i++) {
            cmds = _command_entries;
            user_cmd = *argv++;
            int user_cmd_len = strlen(user_cmd);
            while (NULL != (cmd = *cmds++)) {
                int cmd_name_len = strlen(cmd->name);
                if (user_cmd_len <= cmd_name_len && user_cmd_len >= cmd->min_match) {
                    if (0 == strncmp(cmd->name, user_cmd, user_cmd_len)) {
                        // This command matches
                        disp_commands = false;
                        cmd_help_display(cmd, HELP_DISP_LONG);
                        break;
                    }
                }
            }
            if (disp_commands) {
                ui_term_printf("Unknown: '%s'\n", user_cmd);
            }
        }
    }
    if (disp_commands) {
        // List all of the commands with thier usage.
        ui_term_puts("Commands:\n");
        cmds = _command_entries;
        while (NULL != (cmd = *cmds++)) {
            bool dot_cmd = ('.' == *(cmd->name));
            if (!dot_cmd || (dot_cmd && disp_hidden)) {
                cmd_help_display(cmd, HELP_DISP_NAME);
            }
        }
    }

    return (0);
}

static int _cmd_keys(int argc, char** argv, const char* unparsed) {
    if (argc > 1) {
        cmd_help_display(&_cmd_keys_entry, HELP_DISP_USAGE);
        return (-1);
    }
    ui_term_printf("^H  : Backspace (same as Backspace key on most terminals).\n");
    ui_term_printf("^R  : Refresh the terminal screen.\n");
    ui_term_printf("ESC : Clear the input line.\n");

    return (0);
}

static void _cmd_ps_print(const proc_status_accum_t* ps, int corenum) {
    int uaf = (ONE_SECOND_US - (ps->t_active + ps->t_idle + ps->t_msg_retrieve)) / 1000;
    ui_term_printf("Core %d: Temp:%0.1f Retrieved:%u Idle:%u Active-us:%lu Idle-us:%lu Retrieve-us:%lu ?-ms:%d Intr:0x%0.8x\n",
        corenum, ps->core_temp, ps->retrieved, ps->idle, ps->t_active, ps->t_idle, ps->t_msg_retrieve, uaf, ps->interrupt_status);
}

static int _cmd_proc_status(int argc, char** argv, const char* unparsed) {
    if (argc > 2) {
        // We only take a single argument.
        cmd_help_display(&_cmd_proc_status_entry, HELP_DISP_USAGE);
        return (-1);
    }
    proc_status_accum_t ps0, ps1;
    int smwc;
    bool showmsgs = false;
    if (argc > 1) {
        // They entered an option and/or command names
        if (strcmp("-m", argv[1]) == 0 || strcmp("--msg", argv[1]) == 0) {
            showmsgs = true;
        }
        else {
            // Not our argument.
            cmd_help_display(&_cmd_proc_status_entry, HELP_DISP_USAGE);
            return (-1);
        }
    }

    cmt_proc_status_sec(&ps0, 0);
    cmt_proc_status_sec(&ps1, 1);
    smwc = cmt_sched_msg_waiting();
    _cmd_ps_print(&ps0, 0);
    _cmd_ps_print(&ps1, 1);
    ui_term_printf("Scheduled messages: %d\n", smwc);
    if (smwc > 0) {
        if (showmsgs) {
            uint16_t msgs[SCHEDULED_MESSAGES_MAX];
            cmt_sched_msg_waiting_ids(SCHEDULED_MESSAGES_MAX, msgs);
            for (int i=0; i<SCHEDULED_MESSAGES_MAX; i++) {
                uint16_t id = msgs[i];
                if ((int16_t)id < 0) {
                    break;  // End of messages
                }
                ui_term_printf(" Scheduled Msg: %u [0x%03X]", id, id);
                if (id >= MSG_BACKEND_NOOP && id < MSG_UI_NOOP) {
                    ui_term_printf(" (BE+%u)\n", id - MSG_BACKEND_NOOP);
                }
                else if (id >= MSG_UI_NOOP) {
                    ui_term_printf(" (UI+%u)\n", id - MSG_UI_NOOP);
                }
                else {
                    ui_term_printf("\n");
                }
            }
        }
    }
    else {
        ui_term_printf(" No messages scheduled.\n");
    }
    // Print the PC of the IR PIO state machines
    uint8_t ir_a_sm_pc = pio_sm_get_pc(PIO_IR_BLOCK, PIO_IR_A_SM);
    uint8_t ir_b_sm_pc = pio_sm_get_pc(PIO_IR_BLOCK, PIO_IR_B_SM);
    uint8_t intr_state = 0;
    for (int i=7; i>=0; i--) {
        intr_state |= (pio_interrupt_get(PIO_IR_BLOCK, i) ? 1 : 0);
        if (i > 0) {
            intr_state <<= 1;
        }
    }
    ui_term_printf("IR PIO: Intr:%0.2x - IR-A-PC:%d  IR-B-PC:%d\n", intr_state, ir_a_sm_pc, ir_b_sm_pc);

    return (0);
}


// Internal functions

/**
 * @brief Registered to handle ^R to re-initialize the terminal.
 *
 * @param c Should be ^R
 */
void _handle_reinit_terminal_char(char c) {
    // ^R can be typed if the terminal gets messed up or is connected after scores has started.
    // This re-initializes the terminal.
    cmt_msg_t msg;
    msg.id = MSG_CMD_INIT_TERMINAL;
    msg.data.c = c;
    postUIMsgBlocking(&msg);
}

static void _process_line(char* line) {
    if (*line) {
        char* argv[CMD_LINE_MAX_ARGS];
        memset(argv, 0, sizeof(argv));

        _cmd_state = CMD_PROCESSING_LINE;

    if (!ui_term_last_print_was_nl()) {
        ui_term_puts("\n");
    }

        // Copy the line into a buffer for parsing
        strcpy(_cmdline_parsed, line);

        int argc = parse_line(_cmdline_parsed, argv, CMD_LINE_MAX_ARGS);
        char* user_cmd = argv[0];
        int user_cmd_len = strlen(user_cmd);
        bool command_matched = false;

        if (user_cmd_len > 0) {
            const cmd_handler_entry_t** cmds = _command_entries;
            const cmd_handler_entry_t* cmd;

            while (NULL != (cmd = *cmds++)) {
                int cmd_name_len = strlen(cmd->name);
                if (user_cmd_len <= cmd_name_len && user_cmd_len >= cmd->min_match) {
                    if (0 == strncmp(cmd->name, user_cmd, user_cmd_len)) {
                        // This command matches
                        command_matched = true;
                        _cmd_state = CMD_EXECUTING_COMMAND;
                        cmd->cmd(argc, argv, line);
                        ui_term_last_print_was_nl_set(true);
                        break;
                    }
                }
            }
            if (!command_matched) {
                ui_term_printf("Command not found: '%s'. Try 'help'.\n", user_cmd);
            }
        }
    }
    // Get a command from the user...
    _cmd_state = CMD_COLLECTING_LINE;
    term_color_default();
    if (!ui_term_last_print_was_nl()) {
        ui_term_puts("\n");
    }
    ui_term_printf("%c", CMD_PROMPT);
    ui_term_getline(_process_line);
}


// Public functions

const cmd_state_t cmd_get_state() {
    return _cmd_state;
}

void cmd_help_display(const cmd_handler_entry_t* cmd, const cmd_help_display_format_t type) {
    // ZZZ (not reading from terminal)
    //term_color_pair_t tc = ui_term_color_get();
    term_color_default();
    if (HELP_DISP_USAGE == type) {
        ui_term_puts("Usage: ");
    }
    int name_min = cmd->min_match;
    char* name_rest = ((cmd->name) + name_min);
    // Print the minimum required characters bold and the rest normal.
    term_text_bold();
    // ui_term_printf(format, cmd->name);
    ui_term_printf("%.*s", name_min, cmd->name);
    term_text_normal();
    // See if this is an alias for another command...
    bool alias = (CMD_ALIAS_INDICATOR == *cmd->usage);
    if (!alias) {
        ui_term_printf("%s %s\n", name_rest, cmd->usage);
        if (HELP_DISP_LONG == type || HELP_DISP_USAGE == type) {
            ui_term_printf("  %s\n", cmd->description);
        }
    }
    else {
        const char* aliased_for_name = ((cmd->usage) + 1);
        ui_term_printf("%s  Alias for: %s\n", name_rest, aliased_for_name);
        if (HELP_DISP_NAME != type) {
            // Find the aliased entry
            const cmd_handler_entry_t* aliased_cmd = NULL;
            const cmd_handler_entry_t* cmd_chk;
            const cmd_handler_entry_t** cmds = _command_entries;
            while (NULL != (cmd_chk = *cmds++)) {
                if (strcmp(cmd_chk->name, aliased_for_name) == 0) {
                    aliased_cmd = cmd_chk;
                    break;
                }
            }
            if (aliased_cmd) {
                // Put the terminal colors back, and call this again with the aliased command
                // ZZZ term_color_fg(tc.fg);
                // ZZZ term_color_bg(tc.bg);
                cmd_help_display(aliased_cmd, type);
            }
        }
    }
    // ZZZ term_color_fg(tc.fg);
    // ZZZ term_color_bg(tc.bg);
}

void cmd_module_init() {
    // Register the control character handlers.
    ui_term_register_control_char_handler(CMD_REINIT_TERM_CHAR, _handle_reinit_terminal_char);
    // Start the command processor.
    _process_line(NULL);
}
