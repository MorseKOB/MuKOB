#ifndef __SYSTEM_DEFS_H
#define __SYSTEM_DEFS_H

#include "pico/stdlib.h"

// SPI
// Using SPI 1, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// Note: 'Pins' are the GPIO number, not the physical pins on the device.
#define SPI_DEVICE spi1     // Hardware SPI to use
#define SPI_MOSI 11         // DP-15
#define SPI_MISO 12         // DP-16
#define SPI_SCK  10         // DP-14
#define SPI_CS_ENABLE 0     // LOW
#define SPI_CS_DISABLE 1    // HIGH
// Devices
#define SPI_CS_DISPLAY  13  // DP-17
#define SPI_DC_DISPLAY  3   // DP-05
#define SPI_CS_SDCARD   9   // DP-12
#define SPI_CS_TOUCH    5   // DP-07
//
#define IRQ_KOB_KEY     17  // DP-22
#define IRQ_ROTORY_TURN 14  // DP-19
#define IRQ_ROTORY_SW   6   // DP-09
#define IRQ_TOUCH       4   // DP-06
//
#define ILI9341_RESET_OUT       2   // DP-4
#define ILI9341_BACKLIGHT_OUT   7   // DP-10
#define ROTORY_A_IN             14  // DP-19 - Same as IRQ. 'B" must be the next GPIO (15)
#define ROTORY_B_IN             15  // DP-20 - Must be 1 greater than 'A' in.
#define ROTORY_SW_IN            6   // DP-09 - Same as IRQ.
#define KOB_KEY_IN              17  // DP-22 - Same as IRQ.
#define KOB_SOUNDER_OUT         16  // DP-21
#define OPTIONS_1_IN            18  // DP-24 - Dip switch for options is 1234 - 4 BAUD OFF-OFF=115200 OFF-ON=38400
#define OPTIONS_2_IN            19  // DP-25 - Dip switch for options is 1234 - 3 BAUD ON-OFF=19200 ON-ON=9600
#define OPTIONS_3_IN            20  // DP-26 - Dip switch for options is 1234 - 2 ON = MKOB4 Interface (key-sounder interface)
#define OPTIONS_4_IN            21  // DP-27 - Dip switch for options is 1234 - 1 ON = Debug logging
#define BUZZER_OUT              22  // DP-29 - Buzzer enable (High = ON)

// I2C
// This example will use the default I2C on GPIO4 (SDA) and GPIO5 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c_default
#define I2C_SDA PICO_DEFAULT_I2C_SDA_PIN
#define I2C_SCL PICO_DEFAULT_I2C_SCL_PIN

// Buzzer support
#define BUZZER_OFF 0
#define BUZZER_ON 1

// Display
// Hardware driven values (writes to GPIO)
#define ILI9341_BACKLIGHT_OFF 0
#define ILI9341_BACKLIGHT_ON 1
#define ILI9341_HW_RESET_OFF 1
#define ILI9341_HW_RESET_ON 0
#define ILI9341_DC_DATA 1
#define ILI9341_DC_CMD 0

// KOB support
#define KOB_KEY_CLOSED 1
#define KOB_KEY_OPEN 0
#define KOB_SOUNDER_DEENERGIZED 1
#define KOB_SOUNDER_ENERGIZED 0

// Options bit masks
enum option_t {
    OPTION_BAUD_RATE  = 0x0C,  // 1100 < Mask for the baud rate select option
    OPTION_MKOB_IFACE = 0x02,  // 0010 < Mask for MKOB interface peripheral option
    OPTION_DEBUG      = 0x01,  // 0001 < Mask for Debug functionality option
};

#endif
