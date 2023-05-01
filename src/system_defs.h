#ifndef _SYSTEM_DEFS_H_
#define _SYSTEM_DEFS_H_
#ifdef __cplusplus
extern "C" {
#endif

#define MuKOB_VERSION_INFO "MuKOB v0.1"  // ZZZ get from a central name/version string

#undef putc     // Undefine so the standard macros will not be used
#undef putchar  // Undefine so the standard macros will not be used

#include <hardware/exception.h>
#include "multicore.h"
#include "pico/stdlib.h"

// SPI
//
// MuKOB uses 3 SPI devices; Display, Touch Panel, SD Card
// These are split across the two HW SPIs on the Pico so the display can be
// contolled at the same time the touch is read, or the SD card is read/written.
// This also allows the display to be clocked at a higher rate than the SD card
// can reliably be used with.
//
// Pins can be changed, see the GPIO function select table in the datasheet for
// information on GPIO assignments.
// Note: 'Pins' are the GPIO number, not the physical pins on the device.

// Using SPI 0 for the Touch and SD
#define SPI_TSD_DEVICE spi0     // Hardware SPI to use
#define SPI_TSD_MOSI  3         // DP-5
#define SPI_TSD_MISO 4          // DP-6
#define SPI_TSD_SCK  2          // DP-4

// Using SPI 1, and allocate it to the following GPIO pins
#define SPI_DISPLAY_DEVICE spi1 // Hardware SPI to use
#define SPI_DISPLAY_MOSI 11     // DP-15
#define SPI_DISPLAY_MISO 12     // DP-16
#define SPI_DISPLAY_SCK  10     // DP-14

// Chip select values/levels
#define SPI_CS_ENABLE  0        // LOW
#define SPI_CS_DISABLE 1        // HIGH

// Devices Chip select (and display CMD/Data)
#define SPI_CS_DISPLAY  9       // DP-12
#define SPI_DC_DISPLAY  8       // DP-11
#define SPI_CS_SDCARD   5       // DP-07
#define SPI_CS_TOUCH    6       // DP-09
//
#define IRQ_KOB_KEY     17      // DP-22
#define IRQ_rotary_TURN 14      // DP-19
#define IRQ_rotary_SW   13      // DP-17
#define IRQ_SPACEBAR_SW 28      // DP-34
#define IRQ_TOUCH       7       // DP-10

// PIO
// MuKOB uses PIO for reading the rotary encoder. This narrows the pins that can be used.
#define ROTARY_A_IN             14  // DP-19 - IRQ on same pin. 'B" must be the next GPIO (15)
#define ROTARY_B_IN             15  // DP-20 - Must be 1 greater than 'A' in.

// Other GPIO
#define DISPLAY_RESET_OUT       26  // DP-31
#define DISPLAY_BACKLIGHT_OUT   27  // DP-32
#define KOB_KEY_IN              17  // DP-22 - IRQ on same pin.
#define KOB_SOUNDER_OUT         16  // DP-21
#define OPTIONS_3_IN            20  // DP-26 - Options DIP switch is 123 - 3 USB/BAUD OFF-OFF=115200 OFF-ON=19200
#define OPTIONS_2_IN            19  // DP-25 - Options DIP switch is 123 - 2 USB/BAUD  ON-OFF=9600    ON-ON=USB
#define OPTIONS_1_IN            18  // DP-24 - Options DIP switch is 123 - 1 ON = MKOB4 Interface
#define ROTARY_PB_SW_IN         13  // DP-17 - IRQ on same pin.
#define SPACEBAR_SW             28  // DP-34 - IRQ on same pin.
#define TONE_DRIVE              22  // DP-29 - Speaker drive

// Rotary switch push button state
#define ROTARY_PB_SW_PUSHED 0 // Switch is to GND
#define ROTARY_PB_SW_UNPUSHED 1

// Buzzer/Tone support
#define TONE_OFF 0
#define TONE_ON 1

// Display
// Hardware driven values (writes to GPIO)
#define DISPLAY_BACKLIGHT_OFF 0
#define DISPLAY_BACKLIGHT_ON 1
#define DISPLAY_HW_RESET_OFF 1
#define DISPLAY_HW_RESET_ON 0
#define DISPLAY_DC_DATA 1
#define DISPLAY_DC_CMD 0

// KOB support
#define KOB_KEY_CLOSED 1
#define KOB_KEY_OPEN 0
#define KOB_SOUNDER_DEENERGIZED 1
#define KOB_SOUNDER_ENERGIZED 0

// Options bit masks
typedef enum _option_mask_ {
    OPTION_BAUD_RATE  = 0x0C,  // 1100 < Mask for the baud rate select option
} option_mask_t;

/** @brief Macro to get the size of a structure member */
#define member_size(type, member) sizeof(((type *)0)->member)

#ifdef __cplusplus
}
#endif
#endif // _SYSTEM_DEFS_H_
