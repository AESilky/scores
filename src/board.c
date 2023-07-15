/**
 * Scores Board Initialization and General Functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * This sets up the Pico-W for use for scores.
 * It:
 * 1. Configures the GPIO Pins for the proper IN/OUT, pull-ups, etc.
 * 2. Calls the init routines for Config, UI (Display, Touch, rotary)
 *
 * It provides some utility methods to:
 * 1. Turn the On-Board LED ON/OFF
 * 2. Flash the On-Board LED a number of times
 * 3. Turn the buzzer ON/OFF
 * 4. Beep the buzzer a number of times
 *
*/

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/time.h"
#include "pico/types.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "pico/bootrom.h"
#include "pico/cyw43_arch.h"

#include "system_defs.h"

#include "config/config.h"
#include "ui/display/oled1306_i2c/display_oled1306.h"
#include "board.h"
#include "debug_support.h"
#include "cmt/multicore.h"
#include "net/net.h"
#include "ui/display/score_panel/panel.h"
#include "ui/term/term.h"
#include "util/util.h"

// Internal function declarations

static int _format_printf_datetime(char* buf, size_t len);

/**
 * @brief Initialize the board
 *
 * This sets up the GPIO for the proper direction (IN/OUT), pull-ups, etc.
 * This calls the init for each of the devices/subsystems.
 * If all is okay, it returns 0, else non-zero.
 *
 * Although each subsystem could (some might argue should) configure thier own Pico
 * pins, having everything here makes the overall system easier to understand
 * and helps assure that there are no conflicts.
*/
int board_init() {
    int retval = 0;

    stdio_init_all();

    sleep_ms(50); // Ok to `sleep` as msg system not started

    // Initialize the terminal library
    term_module_init();

    // Initialize the board RTC. It will be set correctly later when we
    // have WiFi and can make a NTP call.
    // Start on Sunday the 1st of January 2023 00:00:01
    datetime_t t = {
            .year = 2023,
            .month = 01,
            .day = 01,
            .dotw = 0, // 0 is Sunday
            .hour = 00,
            .min = 00,
            .sec = 01
    };
    rtc_init();
    rtc_set_datetime(&t);
    // clk_sys is >2000x faster than clk_rtc, so datetime is not updated immediately when rtc_set_datetime() is called.
    // tbe delay is up to 3 RTC clock cycles (which is 64us with the default clock settings)
    sleep_us(100);

    retval = cyw43_arch_init();
    if (retval) {
        error_printf(true, "WiFi init failed");
        return retval;
    }
    cyw43_arch_enable_sta_mode();

    // Chip selects for the SPI paripherals
    gpio_set_function(SPI_SDCARD_CS,   GPIO_FUNC_SIO);
    // Chip selects are active-low, so we'll initialize them to a driven-high state
    gpio_set_dir(SPI_SDCARD_CS, GPIO_OUT);
    // Signal drive strengths
    gpio_set_drive_strength(SPI_SDCARD_SCK, GPIO_DRIVE_STRENGTH_2MA);   // Single device connected
    gpio_set_drive_strength(SPI_SDCARD_MOSI, GPIO_DRIVE_STRENGTH_2MA);  // Single device connected
    gpio_set_drive_strength(SPI_SDCARD_CS, GPIO_DRIVE_STRENGTH_2MA);    // CS goes to a single device
    // Initial output state
    gpio_put(SPI_SDCARD_CS, SPI_CS_DISABLE);
    // SPI 1 initialization for the SD card. Use SPI at 2.2MHz.
    spi_init(SPI_SDCARD_DEVICE, 2200 * 1000);
    gpio_set_function(SPI_SDCARD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SDCARD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SDCARD_MISO, GPIO_FUNC_SPI);

    // I2C Initialisation.
    i2c_init(PICO_DEFAULT_I2C, 400 * 1000);
    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // GPIO Outputs (other than chip-selects)
    //    Tone drive
    gpio_set_function(TONE_DRIVE,   GPIO_FUNC_SIO);
    gpio_set_dir(TONE_DRIVE, GPIO_OUT);
    gpio_set_drive_strength(TONE_DRIVE, GPIO_DRIVE_STRENGTH_4MA);
    gpio_put(TONE_DRIVE, TONE_OFF);
    //    Digit 7-Segment enables
    gpio_set_function(PANEL_DIGIT_SEG_A_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_A_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_A_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_A_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_B_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_B_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_B_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_B_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_C_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_C_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_C_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_C_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_D_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_D_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_D_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_D_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_E_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_E_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_E_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_E_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_F_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_F_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_F_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_F_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_G_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_G_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_G_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_G_GPIO, PANEL_DIGIT_SEG_OFF);
    gpio_set_function(PANEL_DIGIT_SEG_P_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_SEG_P_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_SEG_P_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_SEG_P_GPIO, PANEL_DIGIT_SEG_OFF);
    //    Digit enables
    gpio_set_function(PANEL_DIGIT_A10_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_A10_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_A10_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_A10_GPIO, PANEL_DIGIT_OFF);
    gpio_set_function(PANEL_DIGIT_A01_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_A01_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_A01_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_A01_GPIO, PANEL_DIGIT_OFF);
    gpio_set_function(PANEL_DIGIT_B10_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_B10_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_B10_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_B10_GPIO, PANEL_DIGIT_OFF);
    gpio_set_function(PANEL_DIGIT_B01_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_B01_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_B01_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_B01_GPIO, PANEL_DIGIT_OFF);
    gpio_set_function(PANEL_DIGIT_C10_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_C10_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_C10_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_C10_GPIO, PANEL_DIGIT_OFF);
    gpio_set_function(PANEL_DIGIT_C01_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_C01_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_C01_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_C01_GPIO, PANEL_DIGIT_OFF);
    gpio_set_function(PANEL_DIGIT_IND_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(PANEL_DIGIT_IND_GPIO, GPIO_OUT);
    gpio_set_drive_strength(PANEL_DIGIT_IND_GPIO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PANEL_DIGIT_IND_GPIO, PANEL_DIGIT_OFF);

    // GPIO Inputs
    //    IR Detectors
    gpio_init(IR_A);
    gpio_set_dir(IR_A, GPIO_IN);
    gpio_init(IR_B);
    gpio_set_dir(IR_B, GPIO_IN);
    //    User Input Switch
    gpio_init(USER_INPUT_SW);
    gpio_set_dir(USER_INPUT_SW, GPIO_IN);
    gpio_pull_up(USER_INPUT_SW);


    // Check the user input switch to see if it's pressed during startup.
    //  If yes, set 'debug_mode_enabled'
    if (user_switch_pressed()) {
        debug_mode_enable(true);
    }

    // Initialize hardware AD converter, enable onboard temperature sensor and
    //  select its channel.
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Inputs 0-3 are GPIO pins, 4 is the built-in temp sensor

    // Get the configuration
    config_module_init();
    const config_sys_t* system_cfg = config_sys();

    if (system_cfg->is_set) {
        // Make an NTP call to get the actual time and set the RTC correctly
        // This also initializes the network subsystem
        wifi_set_creds(system_cfg->wifi_ssid, system_cfg->wifi_password);
        network_update_rtc(system_cfg->tz_offset);
        // Give it time to make a NTP call
        sleep_ms(1000); // Ok to `sleep` as msg system not started
    }
    // Now read the RTC and print it
    char datetime_buf[256];
    rtc_get_datetime(&t);
    strdatetime(datetime_buf, sizeof(datetime_buf), &t, SDTC_LONG_TXT_ON | SDTC_TIME_24HOUR);
    printf("RTC set from NTP call - it is %s\n", datetime_buf);

    // Initialize the display
    disp_module_init();

    // Initialize the multicore subsystem
    multicore_module_init();

    puts("\033[32mScores says hello!\033[0m");

    return(true);
}

void boot_to_bootsel() {
    reset_usb_boot(0, 0);
}

static void _tone_sound_pattern_cont(void *user_data) {
    tone_on(false);
}
void tone_sound_pattern(int ms) {
    tone_on(true);
    if (!cmt_message_loop_0_running()) {
        sleep_ms(ms);
        _tone_sound_pattern_cont(NULL);
    }
    else {
        cmt_sleep_ms(ms, _tone_sound_pattern_cont, NULL);
    }
}

void tone_on(bool on) {
    gpio_put(TONE_DRIVE, (on ? TONE_ON : TONE_OFF));
}

void _tone_on_off_cont(void* data) {
    int32_t *pattern = (int32_t*)data;
    tone_on_off(pattern);
}
void tone_on_off(const int32_t *pattern) {
    while (*pattern) {
        tone_sound_pattern(*pattern++);
        int off_time = *pattern++;
        if (off_time == 0) {
            return;
        }
        if (!cmt_message_loop_0_running()) {
            sleep_ms(off_time);
        }
        else {
            cmt_sleep_ms(off_time, _tone_on_off_cont, (void*)pattern);
        }
    }
}

static void _led_flash_cont(void* user_data) {
    led_on(false);
}
void led_flash(int ms) {
    led_on(true);
    if (!cmt_message_loop_0_running()) {
        sleep_ms(ms);
        _led_flash_cont(NULL);
    }
    else {
        cmt_sleep_ms(ms, _led_flash_cont, NULL);
    }
}

void led_on(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

void _led_on_off_cont(void* user_data) {
    int32_t* pattern = (int32_t*)user_data;
    led_on_off(pattern);
}
void led_on_off(const int32_t *pattern) {
    while (*pattern) {
        led_flash(*pattern++);
        int off_time = *pattern++;
        if (off_time == 0) {
            return;
        }
        if (!cmt_message_loop_0_running()) {
            sleep_ms(off_time);
        }
        else {
            cmt_sleep_ms(off_time, _led_on_off_cont, (void*)pattern);
        }
    }
}

uint32_t now_ms() {
    return (us_to_ms(time_us_64()));
}

uint64_t now_us() {
    return (time_us_64());
}

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float onboard_temp_c() {
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    return (tempC);
}

float onboard_temp_f() {
    return (onboard_temp_c() * 9 / 5 + 32);
}

bool user_switch_pressed() {
    return (gpio_get(USER_INPUT_SW) == USER_SW_CLOSED);
}

int _format_printf_datetime(char* buf, size_t len) {
    datetime_t t;
    rtc_get_datetime(&t);
    return (snprintf(buf, len, "%02d-%02d-%04d %02d:%02d:%02d", t.month, t.day, t.year, t.hour, t.min, t.sec));
}

void debug_printf(bool incl_dts, const char* format, ...) {
    if (debug_mode_enabled()) {
        char buf[512];
        int index = 0;
        if (incl_dts) {
            index = _format_printf_datetime(buf, sizeof(buf));
        }
        index += snprintf(&buf[index], sizeof(buf) - index, " DEBUG: ");
        va_list xArgs;
        va_start(xArgs, format);
        index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
        printf(buf);
        va_end(xArgs);
    }
}

void error_printf(bool inc_dts, const char* format, ...) {
    char buf[512];
    int index = 0;
    if (inc_dts) {
        index = _format_printf_datetime(buf, sizeof(buf));
    }
    index += snprintf(&buf[index], sizeof(buf) - index, "\033[91m ERROR: ");
    va_list xArgs;
    va_start(xArgs, format);
    index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
    va_end(xArgs);
    printf("%s\033[0m", buf);
}

void info_printf(bool incl_dts, const char* format, ...) {
    char buf[512];
    int index = 0;
    if (incl_dts) {
        index = _format_printf_datetime(buf, sizeof(buf));
    }
    index += snprintf(&buf[index], sizeof(buf) - index, " INFO: ");
    va_list xArgs;
    va_start(xArgs, format);
    index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
    va_end(xArgs);
    printf(buf);
}

void warn_printf(bool inc_dts, const char* format, ...) {
    char buf[512];
    int index = 0;
    if (inc_dts) {
        index = _format_printf_datetime(buf, sizeof(buf));
    }
    index += snprintf(&buf[index], sizeof(buf) - index, " WARN: ");
    va_list xArgs;
    va_start(xArgs, format);
    index += vsnprintf(&buf[index], sizeof(buf) - index, format, xArgs);
    va_end(xArgs);
    printf(buf);
}

