#ifndef _SYSTEM_DEFS_H_
#define _SYSTEM_DEFS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "pico/stdlib.h"

#define scores_VERSION_INFO "Scores v0.1"  // ZZZ get from a central name/version string

/**
 * @brief Flag indicating that the board is a Pico-W
 */
#ifdef PICO_DEFAULT_LED_PIN
    #define BOARD_IS_PICO
#else
    #define BOARD_IS_PICOW
#endif


#undef putc     // Undefine so the standard macros will not be used
#undef putchar  // Undefine so the standard macros will not be used

#include "hardware/exception.h"
#include "hardware/pio.h"
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

// Using SPI 0 for the SD Card and Display
#define SPI_DISP_SDC_DEVICE spi0    // Hardware SPI to use
#define SPI_DISP_SDC_SCK     2      // DP-4
#define SPI_DISP_SDC_MOSI    3      // DP-5
#define SPI_DISP_SDC_MISO    4      // DP-6
#define SPI_DISP_CS          6      // DP-9
#define SPI_DISP_CD          7      // DP-10
#define SPI_SDC_CS           5      // DP-7

// Chip select values/levels
#define SPI_CS_ENABLE        0      // LOW
#define SPI_CS_DISABLE       1      // HIGH

// PIO Blocks
#define PIO_PANEL_DRIVE_BLOCK   pio0        // PIO Block 0 is used to drive the panel
#define PIO_PANEL_DRIVE_SM      0           // State Machine 0 is used for the panel
#define PIO_IR_BLOCK            pio1        // PIO Block 1 is used to read the IR ports
#define PIO_IR_IRQ              PIO1_IRQ_0  // PIO IRQ to use for the IR
#define PIO_IR_A_SM             0           // State Machine 0 is used to read the front IR (A)
#define PIO_IR_B_SM             1           // State Machine 1 is used to read the rear IR (B)

// Segment Enable
#define PANEL_DIGIT_SEG_A_GPIO    15      // DP-20
#define PANEL_DIGIT_SEG_B_GPIO    14      // DP-19
#define PANEL_DIGIT_SEG_C_GPIO    13      // DP-17
#define PANEL_DIGIT_SEG_D_GPIO    12      // DP-16
#define PANEL_DIGIT_SEG_E_GPIO    11      // DP-15
#define PANEL_DIGIT_SEG_F_GPIO    10      // DP-14
#define PANEL_DIGIT_SEG_G_GPIO     9      // DP-12
#define PANEL_DIGIT_SEG_P_GPIO     8      // DP-11 (Decimal Point)

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
#define PANEL_DIGIT_B10_GPIO    18      // DP-24
#define PANEL_DIGIT_B01_GPIO    19      // DP-25
#define PANEL_DIGIT_C10_GPIO    20      // DP-26
#define PANEL_DIGIT_C01_GPIO    21      // DP-27
#define PANEL_DIGIT_IND_GPIO    22      // DP-29

#define PANEL_DIGIT_OFF          0
#define PANEL_DIGIT_ON           1

#define PANEL_PIO_GPIO_BASE     PANEL_DIGIT_SEG_P_GPIO
#define PANEL_PIO_GPIO_COUNT   15

// Other GPIO
#define IR_A_GPIO               27          // DP-32
#define IR_B_GPIO               28          // DP-33
#define TONE_DRIVE              26          // DP-31 - Buzzer drive
#define USER_INPUT_SW           IR_B_GPIO   // DP-33 - Shared with IR-B.
#define SW_BANK1_GPIO           27          // Boards are set up to use IR or Switch Banks
#define SW_BANK1_ADC            1           // Switch banks are read using the ADC
#define SW_BANK2_GPIO           28          // Boards are set up to use IR or Switch Banks
#define SW_BANK2_ADC            2           // Switch banks are read using the ADC

#define IRQ_INPUT_SW            IR_B_GPIO    // DP-33 - Shared with IR-B

// User Input Switch support
#define USER_SW_OPEN             1      // Switch is connected to GND
#define USER_SW_CLOSED           0

// Buzzer/Tone support
#define TONE_OFF 0
#define TONE_ON  1

// User input controls
#define SWITCH_LONGPRESS_DEFAULT 800 // 0.8 seconds (800ms) is considered a 'long press'
#define SWITCH_REPEAT_MS 250 // If a switch is long-pressed, repeat it every 1/4 second.

#ifdef __cplusplus
}
#endif
#endif // _SYSTEM_DEFS_H_
