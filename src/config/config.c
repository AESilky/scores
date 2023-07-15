/**
 * Scores Configuration functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <stdlib.h>

#include "config.h"
#include "config_fops.h"
#include "config_hndlr.h"

#include "board.h"
#include "cmt/cmt.h"
#include "ui/ui_term.h"
#include "util/util.h"

#include "string.h"

#define _CFG_VERSION_KEY "cfg_version"

#define _CFG_MEM_MARKER_ 3224 // *Magic* & *Air*

/**
 * @brief Holds the 'magic' marker and a config structure to safegaurd free'ing.
 *
 * A config object holds values that are also malloc'ed, and therefore
 * need to be free'ed when the config object is free'ed. To safeguard
 * against a client simply free'ing the config object this structure is malloc'ed
 * and initialized, and then the contained config object is made available to clients.
 * This prevents clients from accedentally free'ing the config object
 * (using `free(config*)`), as it will cause a fault. Rather, the `config_free()`
 * method must be used. It correctly free's the additional malloc'ed objects as well
 * as the main object.
 */
typedef struct _CFG_W_MARKER_ {
    uint16_t marker;
    config_t config;
} _cfg_w_marker_t;

static config_sys_t _system_cfg = { 1, false, 0.0, -1, NULL, NULL };
static uint16_t _sys_not_init_flags;

static int _current_cfg_number;
static config_t* _current_cfg;

static cmt_msg_t _msg_config_changed = { MSG_CONFIG_CHANGED, {0} };


// ============================================================================
// Public
// ============================================================================

int config_boot_number() {
    return _current_cfg_number;
}

extern config_t* config_clear(config_t* cfg) {
    if (cfg) {
        cfg->cfg_version = CONFIG_VERSION;
        if (cfg->name) {
            free(cfg->name);
            cfg->name = NULL;
        }
        cfg->sound = false;
    }
    return (cfg);
}

config_t* config_copy(config_t* cfg_dest, const config_t* cfg_source) {
    if (cfg_dest && cfg_source) {
        config_clear(cfg_dest); // Assure that alloc'ed values are freed
        cfg_dest->sound = cfg_source->sound;
    }
    return (cfg_dest);
}

const config_t* config_current() {
    return _current_cfg;
}

config_t* config_current_for_modification() {
    return _current_cfg;
}

int config_current_number() {
    return _current_cfg_number;
}

void config_free(config_t* cfg) {
    if (cfg) {
        // If this is a config object there is a marker one byte before the beginning.
        // This is to keep it from accidentally being freed directly by `free`, as there
        // are contained structures that also need to be freed.
        _cfg_w_marker_t* cfgwm = (_cfg_w_marker_t*)((uint8_t*)cfg - (sizeof(_cfg_w_marker_t) - sizeof(config_t)));
        if (cfgwm->marker == _CFG_MEM_MARKER_) {
            // Okay, we can free things up...
            // First, free allocated values
            config_clear(cfg);
            // Now free up the main config structure
            free(cfgwm);
        }
    }
}

void config_indicate_changed() {
    postBothMsgBlocking(&_msg_config_changed);
}

extern bool config_load(int config_num) {
    FRESULT fr;

    config_t* cfg = config_new(NULL);
    fr = cfo_read_cfg(cfg, config_num);
    if (FR_OK != fr) {
        ui_term_printf("Could not load configuration %d. (%d)\n", config_num, fr);
        config_free(cfg);
        return (false);
    }
    config_copy(_current_cfg, cfg);
    _current_cfg_number = config_num;
    config_free(cfg);
    config_indicate_changed();

    return (true);
}

void config_make_current(config_t* new_config) {
    if (new_config) {
        config_free(_current_cfg);
        _current_cfg = new_config;
        config_indicate_changed();
    }
}

void config_make_current_w_num(config_t* new_config, int config_num) {
    if (new_config && config_num > 0 && config_num < 10) {
        config_free(_current_cfg);
        _current_cfg = new_config;
        _current_cfg_number = config_num;
        config_indicate_changed();
    }
}

config_t* config_new(const config_t* init_values) {
    // Allocate memory for a config structure, 'mark' it, and
    // set initial values.
    size_t size = sizeof(_cfg_w_marker_t);
    _cfg_w_marker_t* cfgwm = calloc(1, size);
    config_t* cfg = NULL;
    if (NULL != cfgwm) {
        cfgwm->marker = _CFG_MEM_MARKER_;
        cfg = &(cfgwm->config);
        if (NULL != init_values) {
            cfg->cfg_version = init_values->cfg_version;
            cfg->name = str_value_create(init_values->name);
            cfg->sound = init_values->sound;
        }
    }

    return (cfg);
}

extern bool config_save(int config_num, bool set_as_boot) {
    FRESULT fr;

    fr = cfo_save_cfg(_current_cfg, config_num);
    if (FR_OK != fr) {
        ui_term_printf("Could not save configuration %d. (%d)\n", config_num, fr);
        return (false);
    }
    _current_cfg_number = config_num;

    if (set_as_boot) {
        _system_cfg.boot_cfg_number = config_num;
        cfo_save_sys_cfg(&_system_cfg);
    }

    return (true);
}

const config_sys_t* config_sys() {
    return (&_system_cfg);
}

bool config_sys_is_set() {
    return (_system_cfg.is_set);
}

extern bool config_set_boot(int config_num) {
    bool success = false;

    if (config_num >0 && config_num < 10) {
        int cn = _system_cfg.boot_cfg_number;
        _system_cfg.boot_cfg_number = config_num;
        FRESULT fr = cfo_save_sys_cfg(&_system_cfg);
        if (FR_OK != fr) {
            _system_cfg.boot_cfg_number = cn;
            success = false;
        }
    }
    return (success);
}


// ============================================================================
// Initialization
// ============================================================================

void config_module_init() {
    static bool _module_initialized = false;
    if (_module_initialized) {
        panic("config module already initialized.");
    }
    _module_initialized = true;

    // Set default values in the system config
    _system_cfg.cfg_version = CONFIG_VERSION;
    _system_cfg.boot_cfg_number = -1; // Invalid number as flag
    _system_cfg.wifi_ssid = NULL;
    _system_cfg.wifi_password = NULL;

    // Create a config object to use as the current
    config_t* cfg = config_new(NULL);
    _current_cfg = cfg;
    cfg->cfg_version = CONFIG_VERSION;

    // Initialize the item handlers and file operations modules
    config_hndlr_module_init();
    config_fops_module_init();

    // See if we can read the system and user config from the '.cfg' files...
    _sys_not_init_flags = cfo_read_sys_cfg(&_system_cfg);
    if (_sys_not_init_flags) {
        // Something wasn't initialized. See if we got the config number.
        if (_sys_not_init_flags & _SYSCFG_NOT_LOADED) {
            error_printf(false, "Config - Unable to load system configuration.\n");
            return;
        }
        if (_sys_not_init_flags & _SYSCFG_BCN_ID) {
            // Nope... Default to #1
            error_printf(false, "Config - Boot configuration number is not valid. Using '1'.\n");
            _system_cfg.boot_cfg_number = 1;
        }
    }
    _current_cfg_number = _system_cfg.boot_cfg_number;
    FRESULT fr = cfo_read_cfg(cfg, _current_cfg_number);
    if (fr != FR_OK) {
        error_printf(false, "Config - Could not load configuration (#%hu).\n", _current_cfg_number);
    }
}
