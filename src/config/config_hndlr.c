/**
 * Scores Configuration functionaly
 * Read/Write Handlers for file and command processing operations.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */

#include "config_hndlr.h"

#include "config.h"

#include "board.h"
#include "util/util.h"

#include "pico/platform.h"

#include <stdio.h>
#include <stdlib.h>

static int _cih_config_version_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);

static int _cih_sound_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_sound_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_sound =
{ "sound", 'a', "sound", "Use audio (tone)", _cih_sound_reader, _cih_sound_writer };

/**
 * @brief Array of config item class instances.
 * @ingroup config
 *
 * The entries should be in the order that the config lines should be written to the config file.
 */
const cfg_item_handler_class_t* _cfg_handlers[] = {
    &_cihc_sound,
    ((const cfg_item_handler_class_t*)0), // NULL last item to signify end
};

static int _scih_tz_offset_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_tz_offset_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_tz_offset =
{ "tz_offset", "Time zone offset (hours from GMT)", _SYSCFG_TZ_ID, _scih_tz_offset_reader, _scih_tz_offset_writer };

static int _scih_boot_cfg_number_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_boot_cfg_number_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_boot_cfg_number =
{ "bcfg_number", "Config number to load at boot", _SYSCFG_BCN_ID, _scih_boot_cfg_number_reader, _scih_boot_cfg_number_writer };

static int _scih_disp_wrap_back_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_disp_wrap_back_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_disp_wrap_back =
{ "disp_wrap_back", "Display text characters to scan back for EOL wrap", _SYSCFG_DWB_ID, _scih_disp_wrap_back_reader, _scih_disp_wrap_back_writer };

static int _scih_wifi_password_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_wifi_password_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_wifi_password =
{ "wifi_pw", "Wi-Fi password", _SYSCFG_WP_ID, _scih_wifi_password_reader, _scih_wifi_password_writer };

static int _scih_ssid_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_ssid_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_ssid =
{ "wifi_ssid", "Wi-Fi SSID (name)", _SYSCFG_WS_ID, _scih_ssid_reader, _scih_ssid_writer };

static const sys_cfg_item_handler_class_t* _cfg_sys_handlers[] = {
    &_scihc_tz_offset,
    &_scihc_boot_cfg_number,
    &_scihc_wifi_password,
    &_scihc_ssid,
    &_scihc_disp_wrap_back,
    ((const sys_cfg_item_handler_class_t*)0), // NULL last item to signify end
};

static int _cih_sound_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->sound = b;
    retval = 1;

    return (retval);
}

static int _cih_sound_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Use the board sound (tone) for code sounding.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->sound));

    return (len);
}

static int _scih_tz_offset_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    float dv = (float)atof(value);
    sys_cfg->tz_offset = dv;
    retval = 1;

    return (retval);
}

static int _scih_tz_offset_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Timezone offset (hours from GMT).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%.1f", sys_cfg->tz_offset);

    return (len);
}

static int _scih_boot_cfg_number_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    int n = atoi(value);
    if (n > 0 && n < 10) {
        sys_cfg->boot_cfg_number = n;
    }
    else {
        sys_cfg->boot_cfg_number = 0; // Flag as invalid
        error_printf(false, "Config - Invalid value for boot_cfg_number: %s\n", value);
        retval = (-1);
    }

    return (retval);
}

static int _scih_boot_cfg_number_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Config file to load at boot.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hu", sys_cfg->boot_cfg_number);

    return (len);
}

static int _scih_disp_wrap_back_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    int iv = atoi(value);
    sys_cfg->disp_wrap_back = (uint16_t)iv;
    retval = 1;

    return (retval);
}

static int _scih_disp_wrap_back_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Display characters to scan back from EOL for NL wrapping.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", sys_cfg->disp_wrap_back);

    return (len);
}

static int _scih_wifi_password_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    if (sys_cfg->wifi_password) {
        free(sys_cfg->wifi_password);
    }
    sys_cfg->wifi_password = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _scih_wifi_password_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# WiFi password.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", sys_cfg->wifi_password);

    return (len);
}

static int _scih_ssid_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    if (sys_cfg->wifi_ssid) {
        free(sys_cfg->wifi_ssid);
    }
    sys_cfg->wifi_ssid = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _scih_ssid_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# WiFi SSID (name)\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", sys_cfg->wifi_ssid);

    return (len);
}


const cfg_item_handler_class_t** cfg_handlers() {
    return _cfg_handlers;
}

const sys_cfg_item_handler_class_t** cfg_sys_handlers() {
    return _cfg_sys_handlers;
}

void config_hndlr_module_init() {
    static bool _module_initialized = false;
    if (_module_initialized) {
        panic("config_hndlr module already initialized.");
    }
    _module_initialized = true;

}
