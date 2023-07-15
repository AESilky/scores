/**
 * KOB Configuration File Operations
 *
 * This is intended for use by the `config` functionality, rather than as a
 * general interface. It provides the disk (SD) operations.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _KOB_CONFIG_FO_H_
#define _KOB_CONFIG_FO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include "ff.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/types.h"


// extern void config_fops_module_init(const sys_cfg_item_handler_class_t* (*) sys_cfg_handlers[],
//                                     const cfg_item_handler_class_t* (*) cfg_handlers[]);
extern void config_fops_module_init();

/**
 * @brief Read a config file and set the values on a config object.
 * @ingroup config
 *
 * @param cfg The config object to set values on.
 * @param cfg_num The number of the configuration to read.
 * @return F_OK if no errors.
 */
extern FRESULT cfo_read_cfg(config_t* cfg, uint16_t cfg_num);

/**
 * @brief Read from the system config file.
 * @ingroup config
 *
 * @param sys_cfg System config instance to store values into.
 * @return uint16_t Flags for items that could not be set (0 if all could be set).
 */
extern uint16_t cfo_read_sys_cfg(config_sys_t* sys_cfg);

/**
 * @brief Save a configuration.
 *
 * @param cfg The configuration instance to save.
 * @param config_num The number (1-9) to use as the identifier.
 * @return FRESULT File operation result.
 */
extern FRESULT cfo_save_cfg(const config_t* cfg, uint16_t cfg_num);

/**
 * @brief
 *
 * @param sys_cfg The system configuration instance to save.
 * @return FRESULT
 */
extern FRESULT cfo_save_sys_cfg(const config_sys_t* sys_cfg);

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_FO_H_
