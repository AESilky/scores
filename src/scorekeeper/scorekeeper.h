/**
 * Scorekeeper functionality.
 *
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SCOREKEEPER_H_
#define _SCOREKEEPER_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Update the score panel with the current scores
 * @ingroup scorekeeper
 *
 * This updates the A and B digits on the panel with the current values.
 */
extern void scorekeeper_update_panel_scores();

extern void scorekeeper_module_init();

#ifdef __cplusplus
}
#endif
#endif // _SCOREKEEPER_H_
