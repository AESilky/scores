/**
 * KOB Configuration functionaly
 * Read/Write Handlers for file and command processing operations.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _KOB_CONFIG_HNDLR_H_
#define _KOB_CONFIG_HNDLR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "config.h"

/** Bit flags for the key system config items (these must exist to be valid)*/
#define _SYSCFG_VER_ID      0x0001
#define _SYSCFG_BCN_ID      0x0002
#define _SYSCFG_TZ_ID       0x0004
#define _SYSCFG_WP_ID       0x0008
#define _SYSCFG_WS_ID       0x0010
#define _SYSCFG_DWB_ID      0x0020
#define _SYSCFG_IR1_RC      0x0040
#define _SYSCFG_IR2_RC      0x0080
#define _SYSCFG_PANEL_TYPE  0x0100
#define _SYSCFG_NOT_LOADED  0x8000



struct _SYS_CFG_ITEM_HANDLER_CLASS_;
typedef struct _SYS_CFG_ITEM_HANDLER_CLASS_ sys_cfg_item_handler_class_t;

/**
 * @brief System config item handler type. Functions of this type used to process system config file lines.
 * @ingroup config
 *
 * Defines the signature of system config item handlers.
 */
typedef int(*sys_cfg_item_reader_fn)(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, config_sys_t* sys_cfg, const char* value);
typedef int(*sys_cfg_item_writer_fn)(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const config_sys_t* sys_cfg, char* buf, bool full);

struct _SYS_CFG_ITEM_HANDLER_CLASS_ {
    const char* key;
    const char* label;
    const uint16_t id_flag;
    const sys_cfg_item_reader_fn reader;
    const sys_cfg_item_writer_fn writer;
};

struct _CFG_ITEM_HANDLER_CLASS_;
typedef struct _CFG_ITEM_HANDLER_CLASS_ cfg_item_handler_class_t;

/**
 * @brief Config item handler type. Functions of this type used to process config file lines.
 * @ingroup config
 *
 * Defines the signature of config item handlers.
 */
typedef int(*cfg_item_reader_fn)(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
typedef int(*cfg_item_writer_fn)(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf, bool full);

struct _CFG_ITEM_HANDLER_CLASS_ {
    const char* key;
    const char short_opt;
    const char* long_opt;
    const char* label;
    const cfg_item_reader_fn reader;
    const cfg_item_writer_fn writer;
};

/**
 * @brief Get the collection of Config Handlers.
 * @ingroup config
 *
 * @return const cfg_item_handler_class_t** NULL terminated list of CFG Item Handlers.
 */
extern const cfg_item_handler_class_t** cfg_handlers();

/**
 * @brief Get the collection of System Config Handlers.
 * @ingroup config
 *
 * @return const sys_cfg_item_handler_class_t** NULL terminated list of
 *      System CFG Item Handlers.
 */
extern const sys_cfg_item_handler_class_t** cfg_sys_handlers();

/**
 * @brief Initialize the Config Item Handler module.
 * @ingroup config
 */
extern void config_hndlr_module_init();

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_HNDLR_H_
