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

#include "font_7segment.h"

const digsegs_t font_7seg_table[] =
{
    // 0x00 < NULL >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x01 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x02 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x03 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x04 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x05 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x06 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x07 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x08 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x09 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x0A <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x0B <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x0C <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x0D <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x0E <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x0F <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x10 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x11 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x12 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x13 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x14 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x15 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x16 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x17 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x18 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x19 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x1A <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x1B <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x1C <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x1D <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x1E <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x1F <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x20 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x21 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x22 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x23 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x24 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x25 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x26 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x27 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x28 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x29 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x2A <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x2B <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x2C <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x2D <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x2E <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x2F <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x30 < 0 >
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x31 < 1 >
    SEG__ | SEG_B | SEG_C | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x32 < 2 >
    SEG_A | SEG_B | SEG__ | SEG_D | SEG__ | SEG_F | SEG_G | SEG__,

    // 0x33 < 3 >
    SEG_A | SEG_B | SEG_C | SEG_D | SEG__ | SEG__ | SEG_G | SEG__,

    // 0x34 < 4 >
    SEG__ | SEG_B | SEG_C | SEG__ | SEG__ | SEG_F | SEG_G | SEG__,

    // 0x35 < 5 >
    SEG_A | SEG__ | SEG_C | SEG_D | SEG__ | SEG_F | SEG_G | SEG__,

    // 0x36 < 6 >
    SEG_A | SEG__ | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x37 < 7 >
    SEG_A | SEG_B | SEG_C | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x38 < 8 >
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x39 < 9 >
    SEG_A | SEG_B | SEG_C | SEG__ | SEG__ | SEG_F | SEG_G | SEG__,

    // 0x3A < : >
    SEG_A | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG_G | SEG__,

    // 0x3B < ; >
    SEG_A | SEG__ | SEG_C | SEG__ | SEG__ | SEG__ | SEG_G | SEG__,

    // 0x3C < < >
    SEG__ | SEG_B | SEG_C | SEG__ | SEG__ | SEG__ | SEG_G | SEG__,

    // 0x3D < = >
    SEG__ | SEG__ | SEG__ | SEG_D | SEG__ | SEG__ | SEG_G | SEG__,

    // 0x3E < > >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x3F < ? >
    SEG_A | SEG_B | SEG__ | SEG__ | SEG_E | SEG__ | SEG_G | SEG__,

    // 0x40 < @ >
    SEG_A | SEG_B | SEG__ | SEG_D | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x41 < A >
    SEG_A | SEG_B | SEG_C | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x42 < B >
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x43 < C >
    SEG_A | SEG__ | SEG__ | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x44 < D >
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x45 < E >
    SEG_A | SEG__ | SEG__ | SEG_D | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x46 < F >
    SEG_A | SEG__ | SEG__ | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x47 < G >
    SEG_A | SEG__ | SEG_C | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x48 < H >
    SEG__ | SEG_B | SEG_C | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x49 < I >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x4A < J >
    SEG__ | SEG_B | SEG_C | SEG_D | SEG_E | SEG__ | SEG__ | SEG__,

    // 0x4B < K >
    SEG__ | SEG_B | SEG_C | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x4C < L >
    SEG__ | SEG__ | SEG__ | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x4D < M >
    SEG_A | SEG__ | SEG_C | SEG__ | SEG_E | SEG__ | SEG_G | SEG__,

    // 0x4E < N >
    SEG__ | SEG__ | SEG_C | SEG__ | SEG_E | SEG__ | SEG_G | SEG__,

    // 0x4F < O >
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x50 < P >
    SEG_A | SEG_B | SEG__ | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x51 < Q >
    SEG_A | SEG_B | SEG_C | SEG__ | SEG__ | SEG_F | SEG_G | SEG__,

    // 0x52 < R >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG_E | SEG__ | SEG_G | SEG__,

    // 0x53 < S >
    SEG_A | SEG__ | SEG_C | SEG_D | SEG__ | SEG_F | SEG_G | SEG__,

    // 0x54 < T >
    SEG_A | SEG__ | SEG__ | SEG__ | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x55 < U >
    SEG__ | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG__ | SEG__,

    // 0x56 < V >
    SEG__ | SEG_B | SEG__ | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x57 < W >
    SEG__ | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x58 < X >
    SEG__ | SEG_B | SEG_C | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x59 < Y >
    SEG__ | SEG_B | SEG__ | SEG__ | SEG_E | SEG_F | SEG_G | SEG__,

    // 0x5A < Z >
    SEG_A | SEG_B | SEG__ | SEG_D | SEG_E | SEG__ | SEG_G | SEG__,

    // 0x5B <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x5C <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x5D <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x5E <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x5F <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x60 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x61 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x62 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x63 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x64 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x65 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x66 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x67 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x68 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x69 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x6A <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x6B <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x6C <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x6D <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x6E <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x6F <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x70 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x71 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x72 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x73 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x74 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x75 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x76 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x77 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x78 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x79 <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x7A <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x7B <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x7C <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x7D <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x7E <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

    // 0x7F <  >
    SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__ | SEG__,

};
