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

#include "panel/segments7/font_7segment.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Panel Digit/Indicator identifiers (indexes)
 * @ingroup panel
 */
typedef enum _panel_digit_enum {
    PANEL_DIGIT_A10 = 0,
    PANEL_DIGIT_A1 = 1,
    PANEL_DIGIT_B10 = 2,
    PANEL_DIGIT_B1 = 3,
    PANEL_DIGIT_C10 = 4,
    PANEL_DIGIT_C1 = 5,
    PANEL_INDICATORS = 6,
    PANEL_FILL = 7              // Not used. Just to fill out the bits
} panel_digit_t;

/**
 * @brief Panel Indicator enables.
 * @ingroup panel
 */
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

typedef enum _panel_type_ {
    PANEL_NUMERIC       = 0,
    PANEL_LINEAR        = 1,
} panel_type_t;

/**
 * @brief Linear value type (24+ bits right-to-left)
 * 
 * Example:
 * 00000000 00000000 00000000 = 0
 * 00000000 00000000 00000001 = 1
 * 00000000 00000000 00000011 = 2
 * ...
 * 11111111 11111111 11111111 = 24
 */
typedef uint32_t linedots_t; // Type to help control method params


/**
 * @brief Blank (clear) the panel.
 */
extern void panel_blank();

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
extern void panel_C10_set(digsegs_t segments);

/**
 * @brief Set the segments for Digit PT1 (period/time)
 * @ingroup panel
 *
 * @param segments The segment enable bits.
 */
extern void panel_C1_set(digsegs_t segments);

/**
 * @brief Set the segments for both Period/Time digits from an array of segment enables
 * @ingroup panel
 *
 * The segment enables are taken from element 0 for the 10s digit and
 * element 1 or the 1s digit.
 *
 * @param segments Two element array of segment enable values
 */
extern void panel_C_set(digsegs_t segments[]);

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
 * @brief Set the Linear Panel A side.
 * @ingroup panel
 * 
 * Sets the 'dots' in the A side of the linear panel. The dots go
 * from bottom to top as the bits go right to left.
 * 
 * @param dots The dot bits to turn on.
 */
extern void panel_LinearA_set(linedots_t dots);

/**
 * @brief Set the Linear Panel B side.
 * @ingroup panel
 * 
 * Sets the 'dots' in the B side of the linear panel. The dots go
 * from bottom to top as the bits go right to left.
 * 
 * @param dots The dot bits to turn on.
 */
extern void panel_LinearB_set(linedots_t dots);

/**
 * @brief Set a digit to blink fast.
 *
 * @param digit The digit to blink
 */
extern void panel_digit_blink_fast_add(panel_digit_t digit);

/**
 * @brief Stop a digit from blinking fast.
 *
 * @param digit The digit to stop blinking
 */
extern void panel_digit_blink_fast_remove(panel_digit_t digit);

/**
 * @brief Set a digit to blink slow.
 *
 * @param digit The digit to blink
 */
extern void panel_digit_blink_slow_add(panel_digit_t digit);

/**
 * @brief Stop a digit from blinking slow.
 *
 * @param digit The digit to stop blinking
 */
extern void panel_digit_blink_slow_remove(panel_digit_t digit);

/**
 * @brief Linear Dots value from a numeric value.
 * @ingroup panel
 * 
 * @param The value (0-24) to represent as dots.
 * @return The linear dots to turn on.
 */
extern linedots_t panel_linedots_for_value(uint8_t value);

/**
 * @brief Get the type of panel NUMERIC or LINEAR.
 * 
 * @return panel_type_t Panel type
 */
extern panel_type_t panel_type();

/**
 * @brief Initialize the Score Panel.
 * @ingroup panel
 */
extern void panel_module_init(panel_type_t panel_type);

#ifdef __cplusplus
}
#endif
#endif // _SCORE_PANEL_H_
