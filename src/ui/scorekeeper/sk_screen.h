/**
 * Scorekeeper display on the OLED screen.
 *
 *
 * Copyright 2024 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SK_SCREEN_H_
#define _SK_SCREEN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "scorekeeper.h"

#include "pico/types.h"
#include <stdbool.h>

/**
 * @brief Blank (clear) the screen.
 */
extern void skscrn_blank();

/**
 * @brief Set the 'A' value.
 * @ingroup scorekeeper
 *
 * Sets the 2 digit 'A' value.
 *
 * @param value 0-99.
 */
extern void skscrn_A_set(uint8_t value);

/**
 * @brief Set the 'A' value from a string.
 * @ingroup scorekeeper
 *
 * Sets the 2 character 'A' value.
 *
 * @param value Pointer to a character buffer. The first two characters are used.
 */
extern void skscrn_A_set_str(const char *value);

/**
 * @brief Set the 'A' value.
 * @ingroup scorekeeper
 *
 * Sets the 2 digit 'B' value.
 *
 * @param value 0-99.
 */
extern void skscrn_B_set(uint8_t value);

/**
 * @brief Set the 'B' value from a string.
 * @ingroup scorekeeper
 *
 * Sets the 2 character 'B' value.
 *
 * @param value Pointer to a character buffer. The first two characters are used.
 */
extern void skscrn_B_set_str(const char *value);

/**
 * @brief Set the 'PT' (Period/Time) value.
 * @ingroup scorekeeper
 *
 * Sets the 2 digit 'PT' value.
 *
 * @param value 0-99.
 */
extern void skscrn_PT_set(uint8_t value);

/**
 * @brief Set the 'PT' value from a string.
 * @ingroup scorekeeper
 *
 * Sets the 2 character 'Period/Time' value.
 *
 * @param value Pointer to a character buffer. The first two characters are used.
 */
extern void skscrn_PT_set_str(const char *value);

/**
 * @brief Set both Indicator values.
 * @ingroup scorekeeper
 *
 * Sets both indicator values. Indicator A from bits 7-4. Indicator B from bits 3-0.
 *
 * @param value Unsigned byte value.
 */
extern void skscrn_IND_set(uint8_t value);

/**
 * @brief Set Indicator-A value.
 * @ingroup scorekeeper
 *
 * Sets the Indicator-A value from bits 3-0.
 *
 * @param value Unsigned byte value.
 */
extern void skscrn_INDA_set(uint8_t value);

/**
 * @brief Set Indicator-B value.
 * @ingroup scorekeeper
 *
 * Sets the Indicator-B value from bits 3-0.
 *
 * @param value Unsigned byte value.
 */
extern void skscrn_INDB_set(uint8_t value);

/**
 * @brief Initialize the Scorekeeper OLED display screen.
 * @ingroup scorekeeper
 */
extern void sk_screen_module_init(sk_output_mode_t output_mode);


#ifdef __cplusplus
}
#endif
#endif // _SK_SCREEN_H_
