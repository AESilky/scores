/**
 * Scores Configuration Command functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */
#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "config_cmd.h"
#include "config.h"
#include "config_fops.h"
#include "config_hndlr.h"

#include "net/net.h"
#include "ui/ui_term.h"
#include "util/util.h"

 // ============================================================================
 // User command processing
 // ============================================================================

static int _config_cmd_bootcfg(int argc, char** argv, const char* unparsed) {
    int retval = 0;
    int cfg_num = config_current_number();

    if (argc > 1) {
        // The first arg is the number to use or a '.' (meaning current)
        if (strcmp(".", argv[1]) != 0) {
            cfg_num = atoi(argv[1]);
            if (cfg_num < 1 || cfg_num > 9) {
                ui_term_printf("Configuration number must be 1-9\n");
                return (-1);
            }
        }
    }
    retval = config_set_boot(cfg_num);

    return (retval);
}

static int _config_cmd_configure(int argc, char** argv, const char* unparsed) {
    config_t* newcfg = NULL;
    static char buf[512];
    const cfg_item_handler_class_t** handlers = cfg_handlers();
    int retval = -1;

    // If there are parameters, try to set values
    if (argc > 1) {
        // Create a config put set the values into. That way, if there is an error the
        // config in use is left unchanged.
        newcfg = config_new(config_current());
        int argn = 1;
        while (argn < argc) {
            char* arg = argv[argn++];
            char dash = '\000';
            char* dashdash = NULL;
            char* key = NULL;
            char* value = NULL;
            const char* eq = "=";
            handlers = cfg_handlers();
            const cfg_item_handler_class_t* handler = NULL;
            // First, check for our -h/--help
            if (strcmp("-h", arg) == 0 || strcmp("--help", arg) == 0) {
                // Yes - print the options for setting and exit.
                ui_term_printf("Options to set configuration values:\n");
                while (*handlers) {
                    const cfg_item_handler_class_t* handler = *handlers;
                    bool has_dash = ('\040' < handler->short_opt);
                    bool has_dashdash = (*handler->long_opt);
                    // If there aren't any options, the value isn't settable using this command.
                    if (!has_dash && !has_dashdash) {
                        handlers++;
                        continue;
                    }
                    // Get the description from the handler class
                    int len = sprintf(buf, "%s:\n  ", handler->label);
                    // Format the dash option string
                    if (has_dash && has_dashdash) {
                        len += sprintf(buf + len, "-%c value, --%s value, ", handler->short_opt, handler->long_opt);
                    }
                    else if (has_dash && !has_dashdash) {
                        len += sprintf(buf + len, "-%c value, ", handler->short_opt);
                    }
                    else if (!has_dash && has_dashdash) {
                        len += sprintf(buf + len, "--%s value, ", handler->long_opt);
                    }
                    // Add the key=value
                    len += sprintf(buf + len, "%s=value", handler->key);
                    ui_term_printf("%s\n", buf);
                    handlers++;
                }
                goto help_exit;
            }
            // Find the handler for this option/key
            if (*arg == '-') {
                if (*(arg + 1) != '-') {
                    // -opt
                    dash = *(arg + 1);
                }
                else {
                    // --long_opt
                    dashdash = arg + 2;
                }
            }
            else if (strchr(arg, *eq)) {
                // key=value
                key = strtok_r(arg, eq, &arg);
                if (NULL != key) {
                    value = strtok_r(arg, eq, &arg);
                }
            }
            else {
                // No opt, long opt, or key=value
                ui_term_printf(" Invalid input: `%s`\n", arg);
                goto error_exit;
            }
            while (*handlers) {
                handler = *handlers;
                if (key) {
                    if (strcmp(handler->key, key) == 0) {
                        break;
                    }
                }
                else if (dashdash) {
                    if (strcmp(handler->long_opt, dashdash) == 0) {
                        break;
                    }
                }
                else if (handler->short_opt == dash) {
                    break;
                }
                // That handler didn't match. Try the next...
                handler = NULL;
                handlers++;
            }
            if (!handler) {
                // Had an error
                char* reason = (key ? "key" : "option");
                char* errval = (key ? key : (dashdash ? dashdash : (arg + 1)));
                ui_term_printf(" Invalid %s: `%s`\n", reason, errval);
                goto error_exit;
            }
            // if we don't have a value we must have had a -opt or --opt, read the next param for the value.
            char* errkey = (key ? key : arg);
            if (!value) {
                if (argn < argc) {
                    value = argv[argn++];
                }
                else {
                    ui_term_printf(" Missing value for `%s`\n", errkey);
                    goto error_exit;
                }
            }
            retval = handler->reader(handler, newcfg, value);
            handler = NULL; // Clear the handler to move to the next argument (if any)
            if (retval < 0) {
                ui_term_printf(" Invalid value for `%s`: `%s`\n", errkey, value);
                goto error_exit;
            }
        }
        // At this point we've processed all of the supplied arguments. Set the config
        config_make_current(newcfg);
    }

    // List the current configuration
    retval = 0; // If we get here, we can return 'OK'
    const config_t* current_cfg = config_current();
    int current_cfg_number = config_current_number();
    int boot_cfg_number = config_boot_number();
    ui_term_printf("Current Config: %s (%d)  Boot Config: %d\n", current_cfg->name, current_cfg_number, boot_cfg_number);
    // Run through the handlers and have each list the configuration value...
    handlers = cfg_handlers();
    // Find the longest label
    int max_lbl_len = 0;
    while (*handlers) {
        const cfg_item_handler_class_t* handler = *handlers;
        int lbl_len = strlen(handler->label);
        if (lbl_len > max_lbl_len) {
            max_lbl_len = lbl_len;
        }
        handlers++;
    }
    max_lbl_len += 2; // Add indent
    handlers = cfg_handlers();
    while (*handlers) {
        const cfg_item_handler_class_t* handler = *handlers;
        int len = 0;
        handler->writer(handler, current_cfg, buf + len, false);
        ui_term_printf("%*s: %s\n", max_lbl_len, handler->label, buf);
        handlers++;
    }

help_exit:
error_exit: // Free the `newcfg` if one was created.
    config_free(newcfg);

    return (retval);
}

static int _config_cmd_load(int argc, char** argv, const char* unparsed) {
    int retval = 0;
    int cfg_num = config_current_number();

    if (argc > 1) {
        // The first arg is the number to use or a '.' (meaning current)
        if (strcmp(".", argv[1]) != 0) {
            cfg_num = atoi(argv[1]);
            if (cfg_num < 1 || cfg_num > 9) {
                ui_term_printf("Configuration number must be 1-9\n");
                return (-1);
            }
        }
    }
    retval = config_load(cfg_num);
    if (retval) {
        const config_t* current_cfg = config_current();
        ui_term_printf("Loaded - %d:%s\n", cfg_num, current_cfg->name);
    }

    return (retval);
}

static int _config_cmd_save(int argc, char** argv, const char* unparsed) {
    int retval = 0;
    int cfg_num = config_current_number();
    config_t* cfg = config_new(config_current());

    if (argc > 3) {
        ui_term_printf("Too many parameters.\n");
        cmd_help_display(&cmd_save_entry, HELP_DISP_USAGE);
        return (-1);
    }
    if (argc > 1) {
        // The first arg is the number to use or a '.' (meaning current)
        if (strcmp(".", argv[1]) != 0) {
            cfg_num = atoi(argv[1]);
            if (cfg_num < 1 || cfg_num > 9) {
                ui_term_printf("Configuration number must be 1-9\n");
                return (-1);
            }
        }
        if (argc > 2) {
            // The config name. Check for valid characters.
            const char* name = argv[2];
            if (strcmp(cfg->name, name) != 0) {
                int namelen = strlen(name);
                if (namelen > CONFIG_NAME_MAX_LEN) {
                    ui_term_printf("Name can be a maximum of %d characters long.\n", CONFIG_NAME_MAX_LEN);
                    return (-2);
                }
                for (int i = 0; i < namelen; i++) {
                    char c = *(name + i);
                    if (!(isalnum(c) || '_' == c || '-' == c || '.' == c)) {
                        ui_term_printf("Name can only contain letters, numbers, period, dash, and underscore.\n");
                        return (-3);
                    }
                }
                if (cfg->name) {
                    free(cfg->name);
                }
                cfg->name = str_value_create(name);
            }
        }
    }
    FRESULT fr = cfo_save_cfg(cfg, cfg_num);
    if (FR_OK != fr) {
        ui_term_printf("Error saving config.");
        return (-1);
    }
    config_make_current_w_num(cfg, cfg_num);
    ui_term_printf("Saved config %d (%s)\n", cfg_num, cfg->name);

    return (retval);
}

const cmd_handler_entry_t cmd_bootcfg_entry = {
    _config_cmd_bootcfg,
    2,
    "bootcfg",
    "[number|.]",
    "Set the current or a specific configuration as the startup.",
};

const cmd_handler_entry_t cmd_cfg_entry = {
    _config_cmd_configure,
    3,
    "cfg",
    "\001configure",
    NULL,
};

const cmd_handler_entry_t cmd_configure_entry = {
    _config_cmd_configure,
    4,
    "configure",
    "[(optname=value | -<flag>/--<longflag> value) [...]]",
    "List current user configuration. Set configuration value(s).",
};

const cmd_handler_entry_t cmd_load_entry = {
    _config_cmd_load,
    2,
    "load",
    "[(number|.)]",
    "Reload the current config. Load a specific config.",
};

const cmd_handler_entry_t cmd_save_entry = {
    _config_cmd_save,
    2,
    "save",
    "[(number|.) [name]]",
    "Save the current config. Save as number (1-9). Save and name.",
};
