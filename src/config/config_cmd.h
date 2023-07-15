/**
 * KOB Configuration functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _KOB_CONFIG_CMD_H_
#define _KOB_CONFIG_CMD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "ui/cmd/cmd_t.h"

extern const cmd_handler_entry_t cmd_bootcfg_entry;
extern const cmd_handler_entry_t cmd_cfg_entry;
extern const cmd_handler_entry_t cmd_configure_entry;
extern const cmd_handler_entry_t cmd_load_entry;
extern const cmd_handler_entry_t cmd_save_entry;
extern const cmd_handler_entry_t cmd_station_entry;

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_CMD_H_
