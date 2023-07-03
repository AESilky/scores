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
#define SSD_SEGMENT_A       13      // DP-17
#define SSD_SEGMENT_B       12      // DP-16
#define SSD_SEGMENT_C       11      // DP-15
#define SSD_SEGMENT_D       10      // DP-14
#define SSD_SEGMENT_E        9      // DP-12
#define SSD_SEGMENT_F        8      // DP-11
#define SSD_SEGMENT_G        7      // DP-10
#define SSD_SEGMENT_P        6      // DP-9 (Decimal Point)
// Indicator Enable (simply renamed segment enables)
#define IND_TEAM_A_1        SSD_SEGMENT_A
#define IND_TEAM_A_2        SSD_SEGMENT_B
#define IND_TEAM_A_3        SSD_SEGMENT_C
#define IND_TEAM_A_4        SSD_SEGMENT_D
#define IND_TEAM_B_1        SSD_SEGMENT_E
#define IND_TEAM_B_2        SSD_SEGMENT_F
#define IND_TEAM_B_3        SSD_SEGMENT_G
#define IND_TEAM_B_4        SSD_SEGMENT_P

#define SSD_SEGMENT_OFF      1
#define SSD_SEGMENT_ON       0

// Digit Enable
// The digits are:
//  A10,A01 (Team A)
//  B10,B01 (Team B)
//  C10,C01 (Period/Time)
//  Indicator (4 Leds under Team A, 4 Leds under Team B)
#define SSD_DIGIT_A10       16      // DP-21
#define SSD_DIGIT_A01       17      // DP-22
#define SSD_DIGIT_B10       18      // DP-21
#define SSD_DIGIT_B01       19      // DP-21
#define SSD_DIGIT_C10       20      // DP-21
#define SSD_DIGIT_C01       21      // DP-21
#define SSD_DIGIT_IND       22      // DP-21

#define SSD_DIGIT_OFF        1
#define SSD_DIGIT_ON         0

// Other GPIO
#define IR_A                14      // DP-19
#define IR_B                15      // DP-20
#define TONE_DRIVE          27      // DP-32 - Buzzer drive
#define USER_INPUT_SW       28      // DP-34 - IRQ on same pin.

#define IRQ_USER_INPUT_SW   USER_INPUT_SW
#define IRQ_IR_A            IR_A
#define IRQ_IR_B            IR_B

// User Input Switch support
#define USER_SW_OPEN         1      // Switch is connected to GND
#define USER_SW_CLOSED       0

// Buzzer/Tone support
#define TONE_OFF 0
#define TONE_ON  1

#ifdef __cplusplus
}
#endif
#endif // _SYSTEM_DEFS_H_
