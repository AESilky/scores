/**
 * Seven Segment Display Font.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * This is an ASCII font with characters as best as possible using the
 * seven segments and the decimal point.
 *
 * In many (most) cases, the upper and lower case characters are the same,
 * but there are a few that are different. Some type of character will
 * be available for all of the alpha, and of course, the numeric characters.
 *
 * The segments are:
 *
 *  +---A---+
 *  |       |
 *  F       B
 *  |       |
 *  +---G---+
 *  |       |
 *  E       C
 *  |       |
 *  +---D---+ P
 *
 * They are represented in 1 byte:
 * 76543210
 * --------
 * ABCDEFGP
 */
#ifndef _FONT_7SEG_H_
#define _FONT_7SEG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief 7 Segment enable bits.
 *
 * The segments are:
 * A - Bit7
 * B - Bit6
 * C - Bit5
 * D - Bit4
 * E - Bit3
 * F - Bit2
 * G - Bit1
 * P - Bit0 (decimal point)
 *
 */
typedef uint8_t digsegs_t; // Type to help control method params

#define SEG__ 0x00
#define SEG_A 0x80
#define SEG_B 0x40
#define SEG_C 0x20
#define SEG_D 0x10
#define SEG_E 0x08
#define SEG_F 0x04
#define SEG_G 0x02
#define SEG_P 0x01

/** @brief Mask for indexes into the font table */
#define FONT_7SEG_INDEX_MASK 0x7F

extern const digsegs_t font_7seg_table[];

#ifdef __cplusplus
}
#endif
#endif // _FONT_7SEG_H_
