#ifndef _SYSTEM_DEFS_H_
#define _SYSTEM_DEFS_H_
#ifdef __cplusplus
extern "C" {
#endif

#define scores_VERSION_INFO "Scores v0.1"  // ZZZ get from a central name/version string

#undef putc     // Undefine so the standard macros will not be used
#undef putchar  // Undefine so the standard macros will not be used

#include "hardware/exception.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

// SPI
//
// Scores uses SPI-1 for the SD Card
//
// The SPI GPIO pins are also used for segment enables for the 7 segment display.
// Therefore, the 7Seg displays can not be on while the SD card is being
// accessed. The shared GPIO pins must be configured for driving the segments when
// the display digits are going to be driven and configured for SPI when the SD
// card is going to be accessed (read or write)l
//
// Note: 'Pins' are the GPIO number, not the physical pins on the device.
//

// Using SPI 1 for the SD Card
#define SPI_SDCARD_DEVICE   spi1    // Hardware SPI to use
#define SPI_SDCARD_MOSI     11      // DP-15
#define SPI_SDCARD_MISO     12      // DP-16
#define SPI_SDCARD_SCK      10      // DP-14
#define SPI_SDCARD_CS       26      // DP-31

// Chip select values/levels
#define SPI_CS_ENABLE        0      // LOW
#define SPI_CS_DISABLE       1      // HIGH

// Segment Enable
#define PANEL_DIGIT_SEG_A_GPIO    13      // DP-17
#define PANEL_DIGIT_SEG_B_GPIO    12      // DP-16
#define PANEL_DIGIT_SEG_C_GPIO    11      // DP-15
#define PANEL_DIGIT_SEG_D_GPIO    10      // DP-14
#define PANEL_DIGIT_SEG_E_GPIO     9      // DP-12
#define PANEL_DIGIT_SEG_F_GPIO     8      // DP-11
#define PANEL_DIGIT_SEG_G_GPIO     7      // DP-10
#define PANEL_DIGIT_SEG_P_GPIO     6      // DP-9 (Decimal Point)
#define PANEL_DIGIT_SEG_GPIO_SHIFT 6    // GPIO shift to write all segments at once

// Indicator Enable (simply renamed segment enables)
#define PANEL_IND_TEAM_A_1        PANEL_DIGIT_SEG_A
#define PANEL_IND_TEAM_A_2        PANEL_DIGIT_SEG_B
#define PANEL_IND_TEAM_A_3        PANEL_DIGIT_SEG_C
#define PANEL_IND_TEAM_A_4        PANEL_DIGIT_SEG_D
#define PANEL_IND_TEAM_B_1        PANEL_DIGIT_SEG_E
#define PANEL_IND_TEAM_B_2        PANEL_DIGIT_SEG_F
#define PANEL_IND_TEAM_B_3        PANEL_DIGIT_SEG_G
#define PANEL_IND_TEAM_B_4        PANEL_DIGIT_SEG_P

#define PANEL_DIGIT_SEG_OFF      0
#define PANEL_DIGIT_SEG_ON       1

// Digit Enable
// The digits are:
//  A10,A01 (Team A)
//  B10,B01 (Team B)
//  C10,C01 (Period/Time)
//  Indicator (4 Leds under Team A, 4 Leds under Team B)
#define PANEL_DIGIT_A10_GPIO    16      // DP-21
#define PANEL_DIGIT_A01_GPIO    17      // DP-22
#define PANEL_DIGIT_B10_GPIO    18      // DP-21
#define PANEL_DIGIT_B01_GPIO    19      // DP-21
#define PANEL_DIGIT_C10_GPIO    20      // DP-21
#define PANEL_DIGIT_C01_GPIO    21      // DP-21
#define PANEL_DIGIT_IND_GPIO    22      // DP-21
#define PANEL_DIGIT_GPIO_SHIFT  16     // GPIO shift to write all enables at once

#define PANEL_DIGIT_OFF          0
#define PANEL_DIGIT_ON           1

#define PANEL_PWM_SLICE          0
#define PANEL_DIGIT_SEGMENT_GPIO_MASK 0x007F3FC0 // 00000000 01111111 00111111 11000000

// Other GPIO
#define IR_A                    14      // DP-19
#define IR_B                    15      // DP-20
#define TONE_DRIVE              27      // DP-32 - Buzzer drive
#define USER_INPUT_SW           28      // DP-34 - IRQ on same pin.

#define IRQ_USER_INPUT_SW       USER_INPUT_SW
#define IRQ_IR_A                IR_A
#define IRQ_IR_B                IR_B

// User Input Switch support
#define USER_SW_OPEN             1      // Switch is connected to GND
#define USER_SW_CLOSED           0

// Buzzer/Tone support
#define TONE_OFF 0
#define TONE_ON  1

#ifdef __cplusplus
}
#endif
#endif // _SYSTEM_DEFS_H_
