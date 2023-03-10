/**
 * MuKOB Board Initialization and General Functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * This sets up the Pico-W for use for MuKOB.
 * It:
 * 1 Configures the GPIO Pins for the proper IN/OUT
 *
 * It provides some utility methods to:
 * 1. Turn the On-Board LED ON/OFF
 * 2. Flash the On-Board LED a number of times
 * 3. Turn the buzzer ON/OFF
 * 4. Beep the buzzer a number of times
 *
*/
#ifndef _MuKBOARD_H_
#define _MuKBOARD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Initialize the board
 *
 * This sets up the GPIO for the proper direction (IN/OUT), pull-ups, etc.
 * This calls the init for each of the devices/subsystems.
 * If all is okay, it returns true, else false.
*/
int board_init(void);

/**
 * @brief Beep the Buzzer on/off
 *
 * @param ms Milliseconds to turn the buzzer on.
*/
void buzzer_beep(int ms);

/**
 * @brief Turn the buzzer on/off
 *
 * @param on True to turn buzzer on, False to turn it off.
*/
void buzzer_on(bool on);

/**
 * @brief Beep the buzzer on/off/on/off...
 *
 * This beeps the buzzer for times specified by the `pattern` in milliseconds.
 *
 * @param pattern Array of millisend values to beep the buzzer on, off, on, etc.
 *      The last element of the array must be 0.
*/
void buzzer_on_off(int pattern[]);

/**
 * @brief Turn the hardware backlight on or off.
 *
 * @param on True to turn on, false to turn off
 */
void display_backlight_on(bool on);

/**
 * @brief Enable/disable the hardware reset on the display.
 *
 * @param on True to enable, false to disable
 */
void display_reset_on(bool on);

/**
 * @brief Flash the LED on/off
 *
 * @param ms Milliseconds to turn the LED on.
*/
void led_flash(int ms);

/**
 * @brief Turn the LED on/off
 *
 * @param on True to turn LED on, False to turn it off.
*/
void led_on(bool on);

/**
 * @brief Turn the LED on off on off...
 *
 * This flashes the LED for times specified by the `pattern` in milliseconds.
 *
 * @param pattern Array of millisend values to turn the LED on, off, on, etc.
 *      The last element of the array must be 0.
*/
void led_on_off(int32_t pattern[]);

/**
 * @brief Turn the LED on/off based on MorseKOB code array...
 *
 * This flashes the LED for times specified by the `pattern` in milliseconds.
 *
 * @param pattern Array of millisend values to turn the LED on, off, on, etc.
 *      The last element of the array must be 0. Negative values are off time, positive values are on time.
*/
void led_blink_mcode(int32_t *code, uint32_t len);

/**
 * @brief Read the option switch
 *
 * Reads the 4-position option switch, caches and returns
 * the value in the low 4 bits.
 *
 * Use options_value() to get the cached value (avoiding
 * the GPIO read)
 *
 * Bit 0: SW-4 BAUD
 * Bit 1: SW-3 BAUD
 * Bit 2: SW-2 MKOB4 Interface (key-sounder interface)
 * Bit 3: SW-1 Debug information output
*/
uint8_t options_read(void);

/**
 * @brief Get the cached value of an option
 *
 * Use options_read() to actually read the switch and
 * cache the value (requiring a GPIO read)
 *
 * @param See option_t for values
 *
*/
bool option_value(uint opt);


void debug_printf(const char* format, ...) __attribute__((format(_printf_, 1, 2)));
void error_printf(const char* format, ...) __attribute__((format(_printf_, 1, 2)));
void info_printf(const char* format, ...) __attribute__((format(_printf_, 1, 2)));
void warn_printf(const char* format, ...) __attribute__((format(_printf_, 1, 2)));

#ifdef __cplusplus
}
#endif
#endif // _MuKBOARD_H_
