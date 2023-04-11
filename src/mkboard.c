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

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/time.h"
#include "pico/types.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/rtc.h"
#include "pico/cyw43_arch.h"

#include "system_defs.h"

#include "config.h"
#include "display.h"
#include "kob.h"
#include "mkboard.h"
#include "multicore.h"
#include "net.h"
#include "term.h"
#include "util.h"

static uint8_t _options_value = 0;

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

    sleep_ms(50);

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

    // SPI 0 initialization for the touch and SD card. Use SPI at 8MHz.
    spi_init(SPI_TSD_DEVICE, 8000 * 1000);
    gpio_set_function(SPI_TSD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TSD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TSD_MOSI, GPIO_FUNC_SPI);
    // SPI 1 initialization for the display. Use SPI at 20MHz.
    spi_init(SPI_DISPLAY_DEVICE, 20000 * 1000);
    gpio_set_function(SPI_DISPLAY_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISPLAY_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISPLAY_MOSI, GPIO_FUNC_SPI);
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
    gpio_set_drive_strength(SPI_TSD_SCK, GPIO_DRIVE_STRENGTH_4MA);      // Multiple devices connected
    gpio_set_drive_strength(SPI_TSD_MOSI, GPIO_DRIVE_STRENGTH_4MA);     // Multiple devices connected
    gpio_set_drive_strength(SPI_DISPLAY_SCK, GPIO_DRIVE_STRENGTH_2MA);  // SPI Display is a single device
    gpio_set_drive_strength(SPI_DISPLAY_MOSI, GPIO_DRIVE_STRENGTH_2MA); // SPI Display is a single device
    gpio_set_drive_strength(SPI_CS_DISPLAY, GPIO_DRIVE_STRENGTH_2MA);   // CS goes to a single device
    gpio_set_drive_strength(SPI_DC_DISPLAY, GPIO_DRIVE_STRENGTH_2MA);   // DC goes to a single device
    gpio_set_drive_strength(SPI_CS_SDCARD, GPIO_DRIVE_STRENGTH_2MA);    // CS goes to a single device
    gpio_set_drive_strength(SPI_CS_TOUCH, GPIO_DRIVE_STRENGTH_2MA);     // CS goes to a single device
    // Initial output state
    gpio_put(SPI_CS_DISPLAY, SPI_CS_DISABLE);
    gpio_put(SPI_DC_DISPLAY, DISPLAY_DC_DATA);
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
    gpio_set_function(DISPLAY_RESET_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_RESET_OUT, GPIO_OUT);
    gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_ON);           // Hold reset on until rest of board is initialized
    gpio_set_function(DISPLAY_BACKLIGHT_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_BACKLIGHT_OUT, GPIO_OUT);
    gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_OFF);     // No backlight until the display is initialized
    gpio_set_function(SPKR_DRIVE,   GPIO_FUNC_SIO);
    gpio_set_dir(SPKR_DRIVE, GPIO_OUT);
    gpio_put(SPKR_DRIVE, SPEAKER_OFF);
    gpio_set_function(KOB_SOUNDER_OUT,   GPIO_FUNC_SIO);
    gpio_set_dir(KOB_SOUNDER_OUT, GPIO_OUT);
    gpio_set_drive_strength(KOB_SOUNDER_OUT, GPIO_DRIVE_STRENGTH_2MA);
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
    gpio_set_function(ROTORY_SW_IN, GPIO_FUNC_SIO);
    gpio_set_dir(ROTORY_SW_IN, GPIO_IN);
    gpio_set_function(ROTORY_A_IN, GPIO_FUNC_SIO);
    gpio_set_dir(ROTORY_A_IN, GPIO_IN);
    gpio_set_function(ROTORY_B_IN, GPIO_FUNC_SIO);
    gpio_set_dir(ROTORY_B_IN, GPIO_IN);

    // Read and cache the option switch value
    options_read();

    // Get the configuration
    config_module_init();
    const config_sys_t* system_cfg = config_sys();

    if (system_cfg->is_set) {
        // Make an NTP call to get the actual time and set the RTC correctly
        // This also initializes the network subsystem
        wifi_set_creds(system_cfg->wifi_ssid, system_cfg->wifi_password);
        network_update_rtc(system_cfg->tz_offset);
        sleep_ms(1000);  // Give it time to make a NTP call
    }
    // Now read the RTC and print it
    char datetime_buf[256];
    rtc_get_datetime(&t);
    strdatetime(datetime_buf, sizeof(datetime_buf), &t, SDTC_LONG_TXT_ON | SDTC_TIME_24HOUR);
    printf("RTC set from NTP call - it is %s\n", datetime_buf);

    // Initialize the display
    display_reset_on(false);
    sleep_ms(100);
    disp_module_init();
    display_backlight_on(true);

    // Initialize the multicore subsystem
    multicore_module_init();

    puts("\033[32mMuKOB says hello!\033[0m");

    return(true);
}

void buzzer_beep(int ms) {
    buzzer_on(true);
    sleep_ms(ms);
    buzzer_on(false);
}

void buzzer_on(bool on) {
    gpio_put(SPKR_DRIVE, on);
}

void buzzer_on_off(const int pattern[]) {
    for (int i = 0; pattern[i] != 0; i++) {
        buzzer_beep(pattern[i++]);
        int off_time = pattern[i];
        if (off_time == 0) {
            return;
        }
        sleep_ms(off_time);
    }
}

void display_backlight_on(bool on) {
    if (on) {
        gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_ON);
    }
    else {
        gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_OFF);
    }
}

void display_reset_on(bool on) {
    if (on) {
        gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_ON);
    }
    else {
        gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_OFF);
    }
}

void led_flash(int ms) {
    led_on(true);
    sleep_ms(ms);
    led_on(false);
}

void led_on(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
    // ZZZ - Temp energize sounder too
    kob_sounder_energize(on);
    // ZZZ - Temp tone if debug on
    if (option_value(OPTION_DEBUG)) {
        buzzer_on(on);
    }
    else {
        buzzer_on(false);
    }
}

void led_on_off(const int32_t pattern[]) {
    for (int i = 0; pattern[i] != 0; i++) {
        led_flash(pattern[i++]);
        int off_time = pattern[i];
        if (off_time == 0) {
            return;
        }
        sleep_ms(off_time);
    }
}

struct _led_blink_mcode_context {
    uint32_t len;
    uint32_t index;
    alarm_id_t alarm_id;
    int* code;
};
static struct _led_blink_mcode_context _mcode_context = {0, 0, (alarm_id_t)0, (int*)NULL};

int64_t _led_blink_mcode_handler(const alarm_id_t id, void* request_state) {
    if (_mcode_context.alarm_id != 0) {
        cancel_alarm(_mcode_context.alarm_id);
        _mcode_context.alarm_id = 0;
    }
    if (_mcode_context.index == _mcode_context.len) {
        if (_mcode_context.code) {
            free(_mcode_context.code);
            _mcode_context.code = NULL;
        }
        led_on(false);
        return 0;
    }
    int ms = *(_mcode_context.code + _mcode_context.index);
    if (ms == 0) {
        if (_mcode_context.code) {
            free(_mcode_context.code);
            _mcode_context.code = NULL;
        }
        led_on(false);
        return 0;
    }
    _mcode_context.index++;
    led_on(ms > 0);
    _mcode_context.alarm_id = add_alarm_in_ms(abs(ms), _led_blink_mcode_handler, NULL, true);

    return 0;
}

void led_blink_mcode(const int32_t* code, uint32_t len) {
    if (_mcode_context.alarm_id != 0) {
        cancel_alarm(_mcode_context.alarm_id);
        _mcode_context.alarm_id = 0;
        led_on(false);
    }
    if (_mcode_context.code) {
        free(_mcode_context.code);
    }
    _mcode_context.code = malloc(len);
    memcpy(_mcode_context.code, code, len);
    _mcode_context.len = len;
    _mcode_context.index = 0;
    _mcode_context.alarm_id = 0;
    _led_blink_mcode_handler(0, NULL);
}

uint32_t now_ms() {
    return (us_to_ms(time_us_64()));
}

uint64_t now_us() {
    return (time_us_64());
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
    _options_value = opt_value;

    return (opt_value);
}

bool option_value(uint opt) {
    if (_options_value & opt) {
        return true;
    }
    return false;
}

int _format_printf_datetime(char* buf, size_t len) {
    datetime_t t;
    rtc_get_datetime(&t);
    return (snprintf(buf, len, "%02d-%02d-%04d %02d:%02d:%02d", t.month, t.day, t.year, t.hour, t.min, t.sec));
}

void debug_printf(const char* format, ...) {
    if (option_value(OPTION_DEBUG)) {
        char buf[512];
        int index = _format_printf_datetime(buf, sizeof(buf));
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

void info_printf(const char* format, ...) {
    char buf[512];
    int index = _format_printf_datetime(buf, sizeof(buf));
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

