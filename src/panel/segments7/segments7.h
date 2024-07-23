/**
 * 7 Segment display utilities. This provides:
 * - ASCII to SEGMENTS
 * - int to SEGMENTS
 * - fixed point int to SEGMENTS
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _7SEGMENT_H_
#define _7SEGMENT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "font_7segment.h"
#include <stdint.h>

/**
 * @brief Segments for a single digit from a character.
 *
 * @param c Char from 0 to 127
 * @return digsegs_t Segments for a single digit
 */
extern digsegs_t dig1_char(char c);

/**
 * @brief Segments for a single digit from an int.
 *
 * @param n Unsigned char from 0 to 9
 * @return digsegs_t Segments for a single digit
 */
extern digsegs_t dig1_int(uint8_t n);

/**
 * @brief Segments for two digits from an int.
 *
 * @param buf Pointer to a buffer large enough to hold two digits
 * @param n Unsigned char from 0 to 99
 */
extern void dig2_int(digsegs_t* buf, uint8_t n);

/**
 * @brief Segments for two digits from an int with 10's blank if n < 10.
 *
 * @param buf Pointer to a buffer large enough to hold two digits
 * @param n Unsigned char from 0 to 99
 */
extern void dig2_int_b(digsegs_t* buf, uint8_t n);

/**
 * @brief Segments for two digits from an int with 10's blank if n < 10
 * and both blank if n == 0.
 *
 * @param buf Pointer to a buffer large enough to hold two digits
 * @param n Unsigned char from 0 to 99
 */
extern void dig2_int_b(digsegs_t* buf, uint8_t n);

/**
 * @brief Segments for a two digit from a string.
 *
 * Digits that aren't set are cleared.
 *
 * @param buf Pointer to a buffer large enough to hold two digits
 * @param s String to take first two characters from
 * @return Number of digits (0,1,2)
 */
extern int dig2_str(digsegs_t* buf, char* s);


#ifdef __cplusplus
}
#endif
#endif // _7SEGMENT_H_
