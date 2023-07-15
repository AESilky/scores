/**
 * Scoreboard Panel functionality.
 *
 * The panel has 3 pairs of 7 segment displays arranged as:
 * 1. Team A
 * 2. Team B
 * 3. Period/Time
 *
 * It also has 4 indicator LEDs under each of the team scores. These
 * are driven using the segment drives as:
 * - Team A, Indicator 1 = Segment A
 * - Team A, Indicator 2 = Segment B
 * - Team A, Indicator 3 = Segment C
 * - Team A, Indicator 4 = Segment D
 * - Team B, Indicator 1 = Segment E
 * - Team B, Indicator 2 = Segment F
 * - Team B, Indicator 3 = Segment G
 * - Team B, Indicator 4 = Segment P
 *
 * @see `panel.c` for details about driving the panel.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SCORE_PANEL_H_
#define _SCORE_PANEL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "ui/segments7/font_7segment.h"

#include <stdbool.h>
#include <stdint.h>


typedef enum _panel_indicator_enable_enum {
    PANEL_IND_NONE      = 0x00,
    PANEL_IND_1         = 0x08,
    PANEL_IND_2         = 0x04,
    PANEL_IND_3         = 0x02,
    PANEL_IND_4         = 0x01,
    // Additional common combinations
    PANEL_IND_12        = 0x0C,
    PANEL_IND_123       = 0x0E,
    PANEL_IND_1234      = 0x0F
} panel_indicator_enable_t;

/**
 * @brief Allow digits in the panel to be enabled (master off/allow).
 * @ingroup panel
 *
 * Allow/disallow digits in the panel to be enabled. A digit in the
 * panel can only be enabled if this has been called with a value of
 * `true`.
 *
 * @param enable True to allow digits to be enabled. False to disallow enabling.
 */
extern void panel_allow_gpio_write(bool enable);

/**
 * @brief Set the segments for Digit A10
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_A10_set(digsegs_t segments);

/**
 * @brief Set the segments for Digit A1
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_A1_set(digsegs_t segments);

/**
 * @brief Set the segments for both A digits from an array of segment enables
 * @ingroup panel
 *
 * The segment enables are taken from element 0 for the 10s digit and
 * element 1 or the 1s digit.
 *
 * @param segments Two element array of segment enable values
 */
extern void panel_A_set(digsegs_t segments[]);

/**
 * @brief Set the segments for Digit B10
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_B10_set(digsegs_t segments);

/**
 * @brief Set the segments for Digit B1
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_B1_set(digsegs_t segments);

/**
 * @brief Set the segments for both B digits from an array of segment enables
 * @ingroup panel
 *
 * The segment enables are taken from element 0 for the 10s digit and
 * element 1 or the 1s digit.
 *
 * @param segments Two element array of segment enable values
 */
extern void panel_B_set(digsegs_t segments[]);

/**
 * @brief Set the segments for Digit PT10 (period/time)
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_PT10_set(digsegs_t segments);

/**
 * @brief Set the segments for Digit PT1 (period/time)
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_PT1_set(digsegs_t segments);

/**
 * @brief Set the segments for both Period/Time digits from an array of segment enables
 * @ingroup panel
 *
 * The segment enables are taken from element 0 for the 10s digit and
 * element 1 or the 1s digit.
 *
 * @param segments Two element array of segment enable values
 */
extern void panel_PT_set(digsegs_t segments[]);

/**
 * @brief Set the segments for the Indicators (A & B together)
 * @ingroup panel
 *
 * The indicators for both scores (A & B) are controlled by a single
 * set of segments (single 'digit' enable) as follows:
 *   A-1 : Segment-A
 *   A-2 : Segment-B
 *   A-3 : Segment-C
 *   A-4 : Segment-D
 *   B-1 : Segment-E
 *   B-2 : Segment-F
 *   B-3 : Segment-G
 *   B-4 : Segment-P (decimal point)
 *
 * To help make the indicators easier to work with, there are methods
 * to set the A and B indicators separately.
 * @see panel_INDA_set(panel_indicator_enable_t)
 * @see panel_INDB_set(panel_indicator_enable_t)
 *
 * @param segments The segment enable bits.
 */
extern void panel_IND_set(digsegs_t segments);

/**
 * @brief Set the indicators for Score A
 * @ingroup panel
 *
 * This method sets just the indicators for Score A.
 * @see `panel_indicator_enable_t` for indicator bits
 * @see panel_INDB_set(panel_indicator_enable_t)
 *
 * @param indicators The indicators to set (panel_indicator_enable_t)
 */
extern void panel_INDA_set(panel_indicator_enable_t indicators);

/**
 * @brief Set the indicators for Score B
 * @ingroup panel
 *
 * This method sets just the indicators for Score A.
 * @see `panel_indicator_enable_t` for indicator bits
 * @see panel_INDA_set(panel_indicator_enable_t)
 *
 * @param indicators The indicators to set (panel_indicator_enable_t)
 */
extern void panel_INDB_set(panel_indicator_enable_t indicators);


/**
 * @brief Initialize the Score Panel.
 * @ingroup panel
 */
extern void score_panel_module_init();

#ifdef __cplusplus
}
#endif
#endif // _SCORE_PANEL_H_
