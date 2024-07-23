/**
 * @brief Cursor Switches functionality through analog input.
 * @ingroup curswitch
 *
 * This reads analog inputs for values indicating switch presses. The switches are laid out in
 * a cursor pattern (see curswitch.h). Each switch is connected via a voltage divider such that it
 * produces a unique value when pressed. Some combinations can also be detected (with less
 * certainty).
 *
 * Copyright 2024 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "curswitch.h"
#include "board.h"
#include "cmt/cmt.h"

#include "hardware/adc.h"

#include <string.h>

/**
 * Calculated (Measured) Switch voltages:
 *
 * UP = 2.72V ()
 * RT = 2.43V ()
 * LF = 1.98V ()
 * DN = 1.34V ()
 * HM = 0.60V ()
 * EN = 0.00V ()
 *
 * Voltages (calculated) High to Low
 * 3.30 = NONE
 *          - 0.58
 * 2.72 = UP
 *          - 0.29
 * 2.43 = RT
 *          - 0.45
 * 1.98 = LF
 *          - 0.64
 * 1.34 = DN
 *          - 0.74
 * 0.60 = HM
 *          - 0.60
 * 0.00 = EN
 */
// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
#define CONVERSION_FACTOR (3.3f / (1 << 12))
#define ALLOWABLE_DELTA 100  // Values +- 100 indicate the switch
#define SW_NONE_VAL 3800 // This value is well below the max (4095), but well above SW_UP
// The following are the values from the ADC for exact readings of the calculated voltages
#define SW_UP_VAL 3376
#define SW_RT_VAL 3016
#define SW_LF_VAL 2458
#define SW_DN_VAL 1663
#define SW_HM_VAL  745
#define SW_EN_VAL    0

typedef struct _sw_val_minmax_ {
    uint min;
    uint max;
} sw_val_minmax_t;

/** Switch min/max values in order by switch number (-1) */
const sw_val_minmax_t _Sw_MinMax_Vals[] = {
    {SW_LF_VAL-ALLOWABLE_DELTA, SW_LF_VAL+ALLOWABLE_DELTA},
    {SW_RT_VAL-ALLOWABLE_DELTA, SW_RT_VAL+ALLOWABLE_DELTA},
    {SW_UP_VAL-ALLOWABLE_DELTA, SW_UP_VAL+ALLOWABLE_DELTA},
    {SW_DN_VAL-ALLOWABLE_DELTA, SW_DN_VAL+ALLOWABLE_DELTA},
    {SW_HM_VAL-ALLOWABLE_DELTA, SW_HM_VAL+ALLOWABLE_DELTA},
    {SW_EN_VAL, SW_EN_VAL+ALLOWABLE_DELTA},
    };

#define SW_READ_DELAY_MS         2 // Delay between reads to get consistant switch values
#define SW_READ_FAILSAFE_COUNT  40 // Number of times to try reads before giving up
#define SW_READ_REPEAT_COUNT     8 // Number of times required reading the same switch number

static int _sw_bank_readings[SW_BANK_COUNT][SW_READ_REPEAT_COUNT];
static int _sw_bank_read_index[SW_BANK_COUNT];
static int _sw_bank_read_failsafe[SW_BANK_COUNT];

static bool _sw_bank_enabled[SW_BANK_COUNT];
static volatile bool _sw_bank_readinprogress[SW_BANK_COUNT];

/** State for the switches on the Bank. */
static sw_state_t sw_bank_state[SW_BANK_COUNT][SW_COUNT];

// ///////////////////////////////////////////////////////
// Internal functions
// ///////////////////////////////////////////////////////

static void _bank_clear(switch_bank_t bank) {
    int bi = bank + SW_BANK_INDEX_OFFSET;
    uint32_t now = now_ms(); // Get one time for all
    for (int i=0; i<SW_COUNT; i++) {
        sw_bank_state[bi][i].pressed = false;
        sw_bank_state[bi][i].ts_ms = now;
    }
}

/**
 * @brief Update the switch states for a bank based on what is currently pressed.
 *
 * From what is currently pressed and the existing switch states, determine if
 * there are any changes, indicate what is changed, and update the state.
 *
 * @param sw_pressed The switch (or virtual switch) pressed, or 0 for none.
 * @param sw_bank The bank of states
 * @param changes A bool array that is updated to true for each switch changed
 * @return true If there were any changes
 * @return false If no changes
 */
static bool _update_states(int sw_pressed, sw_state_t sw_bank[SW_COUNT], bool changes[]) {
    bool changed = false;
    uint32_t now = now_ms();
    for (int i=0; i<SW_COUNT; i++) {
        sw_state_t *ss = &sw_bank[i];
        int s = i+1; // The state array is 0-based, switches are 1-based
        changes[i] = false;
        if (sw_pressed == s) {
            if (!ss->pressed) {
                changed = true;
                changes[i] = true;
                ss->pressed = true;
                ss->ts_ms = now;
            }
        }
        else if (ss->pressed) {
            // This switch was pressed, and now it isn't
            changed = true;
            changes[i] = true;
            ss->pressed = false;
            ss->ts_ms = now;
        }
    }

    return (changed);
}

/**
 * @brief For the value read, return the switch/combination number that is pressed.
 *
 * Using the min/max values, return the switch/virtual number, or 0 for none,
 * or -1 for undetermined.
 *
 * @param sw_val Value from the ADC
 * @return int Switch number, 0 (none), -1 (undetermined)
 */
static int _whats_pressed(uint sw_val) {
    int sw_num = -1;
    // Check for none
    if (sw_val > SW_NONE_VAL) {
        return 0;
    }
    // Check the ranges
    for (int i = 0; i < SW_COUNT; i++) {
        sw_val_minmax_t minmax = _Sw_MinMax_Vals[i];
        if (sw_val >= minmax.min && sw_val <= minmax.max) {
            sw_num = i+1;
            break;
        }
    }
    return (sw_num);
}

/**
 * Called from `cmt_sleep_ms` to read the bank again. This is done
 * until we get SW_READ_REPEAT_COUNT consistant switch number readings.
 * Note that the readings are the 'switch number' not the specific
 * ADC value.
 * 
 * @param user_data Defined as a pointer, but directly contains the bank number.
 */
void _read_bank_delayed(void* user_data) {
    switch_bank_t bank = (switch_bank_t)user_data;
    int bank_index = bank + SW_BANK_INDEX_OFFSET;
    int sw; // Use an 'int' so we can use non-switch values as flags
    uint sw_val;
    // Check all of the switch number readings to see if they are the same
    bool all_the_same = true; // Hope for the best
    for (int i=0; i<(SW_READ_REPEAT_COUNT-1); i++) {
        if (_sw_bank_readings[bank_index][i] != _sw_bank_readings[bank_index][i+1]) {
            all_the_same = false;
            break; // No need to check any more
        }
    }
    if (!all_the_same) {
        // We need to read again...
        if (--_sw_bank_read_failsafe[bank_index] <= 0) {
            // We couldn't get consistant read values. Print a warning and give up.
            warn_printf(true, "Read switch Bank%d failed to get consistant values.", bank);
            _sw_bank_readinprogress[bank_index] = false;
            return;
        }
        uint bank_adc = (bank == SWBANK1 ? SW_BANK1_ADC : SW_BANK2_ADC);
        adc_select_input(bank_adc);
        sw_val = adc_read();
        sw = _whats_pressed(sw_val);
        if (sw >= 0) {
            _sw_bank_readings[bank_index][_sw_bank_read_index[bank_index]] = sw;
            _sw_bank_read_index[bank_index]++;
            if (_sw_bank_read_index[bank_index] >= SW_READ_REPEAT_COUNT) {
                _sw_bank_read_index[bank_index] = 0;
            }
        }
        cmt_sleep_ms(SW_READ_DELAY_MS, _read_bank_delayed, (void*)bank);
        return;
    }
    //
    // We got consistant switch numbers from the required number of reads. Process the switch states.
    sw = _sw_bank_readings[bank_index][0];
    bool changes[SW_COUNT];
    switch_action_data_t presses[SW_COUNT];
    switch_action_data_t releases[SW_COUNT];
    sw_state_t *bank_state = sw_bank_state[bank_index];
    bool changed = sw >= 0 && _update_states(sw, bank_state, changes);
    if (changed) {
        // Figure out what changed and post messages for the actions.
        cmt_msg_t msg = { MSG_SWITCH_ACTION, {0} };
        debug_printf(false, "curswitch:  (%d) => %d\n", bank, sw);
        for (int i=0; i<SW_COUNT; i++) {
            presses[i].bank = 0;
            releases[i].bank = 0;
            if (changes[i]) {
                sw_state_t ss = sw_bank_state[bank_index][i];
                if (ss.pressed) {
                    presses[i].bank = bank;
                    presses[i].pressed = 1;
                    presses[i].switch_id = i+1;
                }
                else {
                    releases[i].bank = bank;
                    releases[i].pressed = 0;
                    releases[i].switch_id = i+1;
                }
                char* state = (ss.pressed ? "Pressed" : "Released");
                debug_printf(false, "curswitch:  Bank: %d  Switch: %d  %s\n", bank, i+1, state);
            }
        }
        // Post the messages - Releases first.
        for (int i=0; i<SW_COUNT; i++) {
            if (releases[i].bank > 0) {
                msg.data.sw_action.bank = releases[i].bank;
                msg.data.sw_action.pressed = releases[i].pressed;
                msg.data.sw_action.switch_id = releases[i].switch_id;
                postBothMsgNoWait(&msg);
            }
        }
        for (int i=0; i<SW_COUNT; i++) {
            if (presses[i].bank > 0) {
                msg.data.sw_action.bank = presses[i].bank;
                msg.data.sw_action.pressed = presses[i].pressed;
                msg.data.sw_action.switch_id = presses[i].switch_id;
                postBothMsgNoWait(&msg);
            }
        }
    }
    _sw_bank_readinprogress[bank_index] = false;
}

static void _read_bank(switch_bank_t bank) {
    // Start reading the state of the switches for the bank
    // (do this until we get the same switch number consistantly)
    int bank_index = bank + SW_BANK_INDEX_OFFSET;
    _sw_bank_readinprogress[bank_index] = true;
    _sw_bank_read_index[bank_index] = 0; // Start at reading 0
    _sw_bank_read_failsafe[bank_index] = SW_READ_FAILSAFE_COUNT;
    // Clear out the inprocess readings. Use different negative values so they aren't equal initially.
    for (int i=0; i<SW_READ_REPEAT_COUNT; i++) {
        int flag_val = -2 - i;
        _sw_bank_readings[bank_index][i] = flag_val;
    }
    _read_bank_delayed((void*)bank);
}

// ///////////////////////////////////////////////////////
// Public functions
// ///////////////////////////////////////////////////////

const char* curswitch_shortname_for_swid(switch_id_t sw_id) {
    const char* sw;
    switch (sw_id) {
        case SW_NONE:
            sw = "";
            break;
        case SW_UP:
            sw = "UP";
            break;
        case SW_RIGHT:
            sw = "RT";
            break;
        case SW_DOWN:
            sw = "DN";
            break;
        case SW_LEFT:
            sw = "LF";
            break;
        case SW_HOME:
            sw = "HM";
            break;
        case SW_ENTER:
            sw = "EN";
            break;
    }
    return (sw);
}

void curswitch_state(switch_bank_t bank, switch_id_t sw, sw_state_t *state) {
    if (sw == SW_NONE) {
        state->pressed = false;
        state->ts_ms = 0;
    }
    else {
        state->pressed = sw_bank_state[bank+SW_BANK_INDEX_OFFSET][sw+SW_INDEX_OFFSET].pressed;
        state->ts_ms = sw_bank_state[bank+SW_BANK_INDEX_OFFSET][sw+SW_INDEX_OFFSET].ts_ms;
    }
}

bool curswitch_sw_pressed(switch_bank_t bank, switch_id_t sw) {
    sw_state_t state;
    curswitch_state(bank, sw, &state);
    return state.pressed;
}

uint32_t curswitch_sw_pressed_duration(switch_bank_t bank, switch_id_t sw) {
    sw_state_t state;
    curswitch_state(bank, sw, &state);
    uint32_t t = 0;
    if (state.pressed) {
        uint32_t now = now_ms();
        t = now - state.ts_ms;
    }

    return (t);
}

void curswitch_trigger_read() {
    if (_sw_bank_enabled[SWBANK1 + SW_BANK_INDEX_OFFSET] && !_sw_bank_readinprogress[SWBANK1 + SW_BANK_INDEX_OFFSET]) {
        // Read the state of the Bank1 switches
        _read_bank(SWBANK1);
    }
    if (_sw_bank_enabled[SWBANK2 + SW_BANK_INDEX_OFFSET] && !_sw_bank_readinprogress[SWBANK2 + SW_BANK_INDEX_OFFSET]) {
        // Read the state of the Bank2 switches
        _read_bank(SWBANK2);
    }
}

// ///////////////////////////////////////////////////////
// Module Initialization
// ///////////////////////////////////////////////////////

void curswitch_module_init(bool sw_bank1_enabled, bool sw_bank2_enabled) {
    _sw_bank_enabled[SWBANK1 + SW_BANK_INDEX_OFFSET] = sw_bank1_enabled;
    _sw_bank_readinprogress[SWBANK1 + SW_BANK_INDEX_OFFSET] = false;
    _sw_bank_enabled[SWBANK2 + SW_BANK_INDEX_OFFSET] = sw_bank2_enabled;
    _sw_bank_readinprogress[SWBANK2 + SW_BANK_INDEX_OFFSET] = false;

    // Start with all switch states OPEN
    _bank_clear(SWBANK1);
    _bank_clear(SWBANK2);

    // adc_init(); // Not needed. This is done during board initialization

    // Make sure GPIO is high-impedance, no pull-ups etc
    if (sw_bank1_enabled) {
        adc_gpio_init(SW_BANK1_GPIO);
    }
    if (sw_bank2_enabled) {
        adc_gpio_init(SW_BANK2_GPIO);
    }
}
