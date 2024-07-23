/**
 * Configuration functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include "panel/panel.h"

#include <stdbool.h>
#include <stdint.h>

#define CONFIG_NAME_MAX_LEN 15
#define CONFIG_VERSION 1

typedef struct _config_ {
    /** Configuration Version */
    uint16_t cfg_version;
    /** Configuration Name */
    char* name;
    /** Long-Press time in milliseconds */
    uint16_t long_press;
    /** Sound is enabled */
    bool sound;
} config_t;

typedef struct _sys_config_ {
    bool is_set;
    /** Configuration Version */
    uint16_t cfg_version;
    /** Boot Configuration Number (for User config) */
    uint8_t boot_cfg_number;
    /** Infrared input #1 is Remote Control / Else Joy-Switches */
    bool ir1_is_rc;
    /** Infrared input #2 is Remote Control / Else Joy-Switches */
    bool ir2_is_rc;
    /** Panel Type (NUMERIC|LINEAR) */
    panel_type_t panel_type;
    /** Time zone offset from GMT (signed float, like '-8.0') */
    float tz_offset;
    /** Wifi Password */
    char* wifi_password;
    /** Wifi SSID */
    char* wifi_ssid;
    /** Number of characters to loop back for a space to wrap text (adjust for display size) */
    uint16_t disp_wrap_back;
} config_sys_t;

/**
 * @brief Get the boot config number.
 * @ingroup config
 * 
 * @return int The config number to use at boot.
 */
extern int config_boot_number();

/**
 * @brief Clear the values of a configuration instance to the initial values.
 * @ingroup config
 *
 * @param cfg The config instance to clear the values from.
 * @return config_t* Pointer to the destination config instance passed in (for convenience)
 */
extern config_t* config_clear(config_t* cfg);

/**
 * @brief Copy values from one config instance to another.
 * @ingroup config
 *
 * @param cfg_dest An existing config instance to copy values into.
 * @param cfg_source Config instance to copy values from.
 * @return config_t* Pointer to the destination config instance passed in (for convenience)
 */
extern config_t* config_copy(config_t* cfg_dest, const config_t* cfg_source);

/**
 * @brief Get the current configuration.
 * @ingroup config
 *
 * @return config_t* Current configuration.
 */
extern const config_t* config_current();

/**
 * @brief Get the current configuration to be modified.
 * @ingroup config
 *
 * @return config_t* Current configuration.
 */
extern config_t* config_current_for_modification();

/**
 * @brief Get the number (1-9) of the current configuration.
 * @ingroup config
 *
 * @return int
 */
extern int config_current_number();

/**
 * @brief Free a config_t structure previously allowcated with config_new.
 * @ingroup config
 *
 * @see config_new(config_t*)
 *
 * @param config Pointer to the config_t structure to free.
 */
extern void config_free(config_t* config);

/**
 * @brief Post a message to both cores indicating that the configuration has changed.
 */
extern void config_indicate_changed();

/**
 * @brief Load the current config from saved config number.
 * @ingroup config
 *
 * @param config_num The config number to load.
 * @return true Config was loaded.
 */
extern bool config_load(int config_num);

/**
 * @brief Make a new config the current config.
 * @ingroup config
 * 
 * After setting the new config as the current config, the config changed
 * message will be posted to both cores.
 *
 * @param new_config The config to make current (replacing the current config)
 */
extern void config_make_current(config_t* new_config);

/**
 * @brief Make a new config the current and set the config number.
 * @ingroup config
 *
 * After setting the new config as the current config, the config changed
 * message will be posted to both cores.
 *
 * @param new_config The config to make current (replacing the current config)
 * @param config_num The number (1-9) to assign to the config
 */
extern void config_make_current_w_num(config_t* new_config, int config_num);

/**
 * @brief Allocate a new config_t structure. Optionally, initialize values.
 * @ingroup config
 *
 * @see config_free(config_t* config)
 * @param init_values Config structure with initial values, or NULL for an empty config.
 * @return config_t* A newly allocated config_t structure. Must be free'ed with `config_free()`.
 */
extern config_t* config_new(const config_t* init_values);

/**
 * @brief Save the current configuration. Optionally set this as the boot configuration.
 * @ingroup config
 *
 * @param config_num The number to assign to the configuration. Must be 1-9.
 * @param set_as_boot True to set this configuration as the boot configuration.
 * @return bool True if successful.
 */
extern bool config_save(int config_num, bool set_as_boot);

/**
 * @brief Set the configuration number as the boot configuration.
 *
 * @param config_num The configuration number to set as boot. Must be 1-9.
 * @return bool True if successful.
 */
extern bool config_set_boot(int config_num);

/**
 * @brief Get the system configuration.
 * @ingroup config
 *
 * @return const config_sys_t* The system configuration.
 */
extern const config_sys_t* config_sys();

/**
 * @brief Indicates if the system config was read and set
 * @ingroup config
 *
 * @return true System config is available.
 * @return false System config could not be read and isn't valid.
 */
extern bool config_sys_is_set();

/**
 * @brief Initialize the configuration subsystem
 * @ingroup config
 * 
 * @return 0 if succesfull. The 'not initialized' bits on fail.
*/
extern int config_module_init();

#ifdef __cplusplus
}
#endif
#endif // _CONFIG_H_
