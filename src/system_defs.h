#ifndef _SYSTEM_DEFS_H_
#define _SYSTEM_DEFS_H_
#ifdef __cplusplus
extern "C" {
#endif

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
#define SPI_TSD_MISO 4         // DP-6
#define SPI_TSD_SCK  2         // DP-4

// Using SPI 1, and allocate it to the following GPIO pins
#define SPI_DISPLAY_DEVICE spi1    // Hardware SPI to use
#define SPI_DISPLAY_MOSI 11        // DP-15
#define SPI_DISPLAY_MISO 12        // DP-16
#define SPI_DISPLAY_SCK  10        // DP-14

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
#define IRQ_ROTORY_TURN 14      // DP-19
#define IRQ_ROTORY_SW   13      // DP-17
#define IRQ_TOUCH       7       // DP-10

// PIO
// MuKOB uses PIO for reading the rotory encoder. This narrows the pins that can be used.
#define ROTORY_A_IN             14  // DP-19 - IRQ on same pin. 'B" must be the next GPIO (15)
#define ROTORY_B_IN             15  // DP-20 - Must be 1 greater than 'A' in.
#define ROTORY_SW_IN            13  // DP-17 - IRQ on same pin.
#define SPKR_DRIVE              22  // DP-29 - Speaker drive

// Other GPIO
#define DISPLAY_RESET_OUT       26  // DP-31
#define DISPLAY_BACKLIGHT_OUT   27  // DP-32
#define KOB_KEY_IN              17  // DP-22 - IRQ on same pin.
#define KOB_SOUNDER_OUT         16  // DP-21
#define OPTIONS_1_IN            18  // DP-24 - Dip switch for options is 1234 - 4 BAUD OFF-OFF=115200 OFF-ON=38400
#define OPTIONS_2_IN            19  // DP-25 - Dip switch for options is 1234 - 3 BAUD ON-OFF=19200 ON-ON=9600
#define OPTIONS_3_IN            20  // DP-26 - Dip switch for options is 1234 - 2 ON = MKOB4 Interface (key-sounder interface)
#define OPTIONS_4_IN            21  // DP-27 - Dip switch for options is 1234 - 1 ON = Debug logging

// Buzzer support
#define SPEAKER_OFF 0
#define SPEAKER_ON 1

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
enum option_t {
    OPTION_BAUD_RATE  = 0x0C,  // 1100 < Mask for the baud rate select option
    OPTION_MKOB_IFACE = 0x02,  // 0010 < Mask for MKOB interface peripheral option
    OPTION_DEBUG      = 0x01,  // 0001 < Mask for Debug functionality option
};

/** @brief Macro to get the size of a structure member */
#define member_size(type, member) sizeof(((type *)0)->member)

#ifdef __cplusplus
}
#endif
#endif // _SYSTEM_DEFS_H_
