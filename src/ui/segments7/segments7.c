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

#include "segments7.h"

static inline void _clear2digs(digsegs_t* buf) {
    *buf = 0;
    *(buf+1) = 0;
}

extern digsegs_t dig1_char(char c) {
    int i = c & FONT_7SEG_INDEX_MASK;
    digsegs_t segs = font_7seg_table[i];

    return (segs);
}

extern digsegs_t dig1_int(uint8_t n) {
    digsegs_t segs = (n > 9 ? 0 : font_7seg_table['0' + n]);

    return (segs);
}

void dig2_int(digsegs_t* buf, uint8_t n) {
    _clear2digs(buf);
    uint8_t t = n / 10;
    uint8_t o = n % 10;
    *buf = dig1_int(t);
    *(buf+1) = dig1_int(o);

    return;
}

int dig2_str(digsegs_t* buf, char* s) {
    int converted = 0;
    _clear2digs(buf);

    while (*s) {
        *buf = dig1_char(*s);
        buf++;
        s++;
        converted++;
    }

    return (converted);
}
