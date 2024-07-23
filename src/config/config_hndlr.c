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
#include <string.h>

static int _cih_config_version_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);

static int _cih_longpress_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_longpress_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_longpress =
{ "long_press", 'l', "lp", "Long press time (ms)", _cih_longpress_reader, _cih_longpress_writer };
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
    &_cihc_longpress,
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

static int _scih_ir1_rc_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_ir1_rc_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_ir1_rc =
{ "ir1_is_rc", "Infrared #1 is remote control", _SYSCFG_IR1_RC, _scih_ir1_rc_reader, _scih_ir1_rc_writer };

static int _scih_ir2_rc_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_ir2_rc_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_ir2_rc =
{ "ir2_is_rc", "Infrared #2 is remote control", _SYSCFG_IR2_RC, _scih_ir2_rc_reader, _scih_ir2_rc_writer };

static int _scih_panel_type_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_panel_type_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_panel_type =
{ "panel_type", "Panel Type", _SYSCFG_PANEL_TYPE, _scih_panel_type_reader, _scih_panel_type_writer };

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
    &_scihc_ir1_rc,
    &_scihc_ir2_rc,
    &_scihc_panel_type,
    ((const sys_cfg_item_handler_class_t*)0), // NULL last item to signify end
};

static int _cih_longpress_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    int iv = atoi(value);
    cfg->long_press = (uint16_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_longpress_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Long-press time (ms).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", cfg->long_press);

    return (len);
}

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

static int _scih_ir1_rc_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    sys_cfg->ir1_is_rc = b;
    retval = 1;

    return (retval);
}

static int _scih_ir1_rc_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# IR input #1 is for remote control.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(sys_cfg->ir1_is_rc));

    return (len);
}

static int _scih_ir2_rc_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    sys_cfg->ir2_is_rc = b;
    retval = 1;

    return (retval);
}

static int _scih_ir2_rc_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# IR input #2 is for remote control.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(sys_cfg->ir2_is_rc));

    return (len);
}

static int _scih_panel_type_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    if (strcmp(value, "LINEAR") == 0) {
        sys_cfg->panel_type = PANEL_LINEAR;
    }
    else if (strcmp(value, "NUMERIC") == 0) {
        sys_cfg->panel_type = PANEL_NUMERIC;
    }
    else {
        sys_cfg->panel_type = PANEL_NUMERIC; // Shouldn't get here!
    }
    retval = 1;

    return (retval);
}

static int _scih_panel_type_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Panel type (NUMERIC|LINEAR).\n%s=", self->key);
    }
    // format the value we are responsible for
    const char* ptv = (sys_cfg->panel_type == 0 ? "NUMERIC" : "LINEAR");
    len += sprintf(buf + len, "%s", ptv);

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
