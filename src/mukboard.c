/**
 * MuKOB Board Initialization and General Functions.
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 * 
 * This sets up the Pico-W for use for MuKOB. 
 * It:
 * 1. Configures the GPIO Pins for the proper IN/OUT, pull-ups, etc.
 * 2. Calls the init routines for Config, UI (Display, Touch, Rotory)
 * 
 * It provides some utility methods to:
 * 1. Turn the On-Board LED ON/OFF
 * 2. Flash the On-Board LED a number of times
 * 3. Turn the buzzer ON/OFF
 * 4. Beep the buzzer a number of times
 *  
*/
#include <stdio.h>

#include "mukboard.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"
#include "system_defs.h"
#include "config.h"
#include "display.h"

uint8_t __options_value = 0;

/**
 * \brief Initialize the board
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

    sleep_ms(1000);
    
    retval = cyw43_arch_init();
    if (retval) {
        printf("WiFi init failed");
        return retval;
    }

    // SPI initialisation. Use SPI at 1MHz.
    spi_init(SPI_DEVICE, 1000*1000);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    // Chip selects for the SPI paripherals
    gpio_set_function(SPI_CS_DISPLAY,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_DC_DISPLAY,   GPIO_FUNC_SIO);  // Data/Command
    gpio_set_function(SPI_CS_SDCARD,   GPIO_FUNC_SIO);
    gpio_set_function(SPI_CS_TOUCH,   GPIO_FUNC_SIO);
    // Chip selects are active-low, so we'll initialize them to a driven-high state
    gpio_set_dir(SPI_CS_DISPLAY, GPIO_OUT);
    gpio_set_dir(SPI_DC_DISPLAY, GPIO_OUT);
    gpio_set_dir(SPI_CS_SDCARD, GPIO_OUT);
    gpio_set_dir(SPI_CS_TOUCH, GPIO_OUT);
    // Signal drive strengths
    gpio_set_drive_strength(SPI_SCK, GPIO_DRIVE_STRENGTH_4MA);          // Multiple devices connected
    gpio_set_drive_strength(SPI_MOSI, GPIO_DRIVE_STRENGTH_4MA);         // Multiple devices connected
    gpio_set_drive_strength(SPI_CS_DISPLAY, GPIO_DRIVE_STRENGTH_2MA);   // CS goes to a single device
    gpio_set_drive_strength(SPI_DC_DISPLAY, GPIO_DRIVE_STRENGTH_2MA);   // DC goes to a single device
    gpio_set_drive_strength(SPI_CS_SDCARD, GPIO_DRIVE_STRENGTH_2MA);    // CS goes to a single device
    gpio_set_drive_strength(SPI_CS_TOUCH, GPIO_DRIVE_STRENGTH_2MA);     // CS goes to a single device
    // Initial output state
    gpio_put(SPI_CS_DISPLAY, SPI_CS_DISABLE);
    gpio_put(SPI_DC_DISPLAY, ILI9341_DC_DATA);
    gpio_put(SPI_CS_SDCARD, SPI_CS_DISABLE);
    gpio_put(SPI_CS_TOUCH, SPI_CS_DISABLE);
    
    // NOT USING I2C AT THIS TIME.
    //
    // I2C Initialisation. 
    // i2c_init(I2C_PORT, 400*1000);
    // // I2C is "open drain", pull ups to keep signal high when no data is being sent
    // gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);

    // GPIO Outputs (other than chip-selects)
    gpio_set_function(ILI9341_RESET_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(ILI9341_RESET_OUT, GPIO_OUT);
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_OFF);
    gpio_set_function(ILI9341_BACKLIGHT_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(ILI9341_BACKLIGHT_OUT, GPIO_OUT);
    gpio_put(ILI9341_BACKLIGHT_OUT, ILI9341_BACKLIGHT_OFF);
    gpio_set_function(BUZZER_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(BUZZER_OUT, GPIO_OUT);
    gpio_put(BUZZER_OUT, BUZZER_OFF);
    gpio_set_function(KOB_SOUNDER_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(KOB_SOUNDER_OUT, GPIO_OUT);
    gpio_put(KOB_SOUNDER_OUT, KOB_SOUNDER_DEENERGIZED);

    // GPIO Inputs
    gpio_set_function(OPTIONS_1_IN,   GPIO_FUNC_SIO);
    gpio_set_dir(OPTIONS_1_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_1_IN);
    gpio_set_function(OPTIONS_2_IN,   GPIO_FUNC_SIO);
    gpio_set_dir(OPTIONS_2_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_2_IN);
    gpio_set_function(OPTIONS_3_IN,   GPIO_FUNC_SIO);
    gpio_set_dir(OPTIONS_3_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_3_IN);
    gpio_set_function(OPTIONS_4_IN,   GPIO_FUNC_SIO);
    gpio_set_dir(OPTIONS_4_IN, GPIO_IN);
    gpio_pull_up(OPTIONS_4_IN);

    options_read(); // Read and cache the option switch value

    // Timer example code - This example fires off the callback after 2000ms
    //add_alarm_in_ms(2000, alarm_callback, NULL, false);

    // Get the configuration
    config_init();

    // Initialize the display
    disp_init();

    puts("MuKOB says hello!");

    return(true);
}

void buzzer_beep(int ms) {
    buzzer_on(true);
    sleep_ms(ms);
    buzzer_on(false);
}

void buzzer_on(bool on) {
    gpio_put(BUZZER_OUT, on);
}

void buzzer_on_off(int pattern[]) {
    for (int i = 0; pattern[i] != 0; i++) {
        buzzer_beep(pattern[i++]);
        int off_time = pattern[i];
        if (off_time == 0) {
            return;
        }
        sleep_ms(off_time);
    }
}

void led_flash(int ms) {
    led_on(true);
    sleep_ms(ms);
    led_on(false);
}

void led_on(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

void led_on_off(int pattern[]) {
    for (int i = 0; pattern[i] != 0; i++) {
        led_flash(pattern[i++]);
        int off_time = pattern[i];
        if (off_time == 0) {
            return;
        }
        sleep_ms(off_time);
    }
}

uint8_t options_read(void) {
    uint8_t opt_value = 0x00;
    uint8_t opt_bit = gpio_get(OPTIONS_4_IN);
    opt_value |= opt_bit;
    opt_value <<= 1;
    opt_bit = gpio_get(OPTIONS_3_IN);
    opt_value |= opt_bit;
    opt_value <<= 1;
    opt_bit = gpio_get(OPTIONS_2_IN);
    opt_value |= opt_bit;
    opt_value <<= 1;
    opt_bit = gpio_get(OPTIONS_1_IN);
    opt_value |= opt_bit;
    opt_value ^= 0x0F;  // Invert the final value (the switches are tied to GND)
    __options_value = opt_value;

    return (opt_value);
}

bool option_value(uint opt) {
    if (__options_value & opt) {
        return true;
    }
    return false;
}
