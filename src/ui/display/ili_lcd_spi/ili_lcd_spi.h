/**
 * ILI Color LCD functionaly interface through SPI
 *
 * Commom to ILI9341 and ILI9488
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _ILI_LCD_SPI_H_
#define _ILI_LCD_SPI_H_
#ifdef __cplusplus
extern "C" {
#endif

// 24-bit RGB Color to Basic 16 Colors (match original PC VGA system)
//                                      NUM :   R    G    B
//                                      --- : ---  ---  ---
// #define ILI_BLACK       0x000000  //  0 :   0,   0,   0
// #define ILI_BLUE        0x0000AA  //  1 :   0,   0, 170
// #define ILI_GREEN       0x4E9100  //  2 :  78, 145,   0
// #define ILI_CYAN        0x00FAFF  //  3 :   0, 250, 255
// #define ILI_RED         0xAA0000  //  4 : 170,   0,   0
// #define ILI_MAGENTA     0xFF40FF  //  5 : 255,  64, 255
// #define ILI_BROWN       0x641100  //  6 : 100,  17,   0
// #define ILI_WHITE       0xD0D0D0  //  7 : 208, 208, 208
// #define ILI_GREY        0x79795A  //  8 : 121, 121,  90
// #define ILI_LT_BLUE     0x0064FF  //  9 :   0, 100, 255
// #define ILI_LT_GREEN    0x00F900  // 10 :   0, 249,   0
// #define ILI_LT_CYAN     0x73FDFF  // 11 : 115, 253, 255
// #define ILI_ORANGE      0xFF4B02  // 12 : 255,  75,   2
// #define ILI_LT_MAGENTA  0xFF8AD8  // 13 : 255, 138, 218
// #define ILI_YELLOW      0xFF9300  // 12 : 255, 147,   0
// #define ILI_BR_WHITE    0xFFFFFF  // 15 : 255, 255, 255


// 16-bit Color Mode to Basic 16 Colors (similar to PC CGA/EGA/VGA)
//
// 16-bit is R5G6B5 (RGB-16)
// 16-bit from RGB = (R/8)<<11 + (G/4)<<5 + (B/8)
// 16-bit from R5G6B5 = R5*2048 + G6*32 + B5
//
//                                      NUM :   R    G    B  R5 G6 B5
//                                      --- : ---  ---  ---  -- -- --
#define ILI_BLACK       0x0000    //  0 :   0,   0,   0   0  0  0
#define ILI_BLUE        0x0011    //  1 :   0,   0, 136   0  0 17
#define ILI_GREEN       0x4C80    //  2 :  78, 145,   0   9 36  0
#define ILI_CYAN        0x079E    //  3 :   0, 240, 240   0 60 30
#define ILI_RED         0xE000    //  4 : 224,   0,   0  28  0  0
#define ILI_MAGENTA     0xFA1F    //  5 : 255,  64, 255  31 16 31
#define ILI_BROWN       0x6080    //  6 : 100,  17,   0  12  4  0
#define ILI_WHITE       0xB5D2    //  7 : 180, 190, 150  22 46 18
#define ILI_GREY        0x6B49    //  8 : 104, 104,  72  13 26  9
#define ILI_LT_BLUE     0x033F    //  9 :   0, 100, 255   0 25 31
#define ILI_LT_GREEN    0x07E0    // 10 :   0, 255,   0   0 63  0
#define ILI_LT_CYAN     0x77FF    // 11 : 115, 253, 255  14 63 31
#define ILI_ORANGE      0xFA40    // 12 : 255,  75,   2  31 18  0
#define ILI_LT_MAGENTA  0xFC5B    // 13 : 255, 138, 218  31 34 27
#define ILI_YELLOW      0xFFEA    // 12 : 255, 255,  85  31 63 10
#define ILI_BR_WHITE    0xFFFF    // 15 : 255, 255, 255  31 63 31

typedef unsigned short rgb16_t; // R5G6B5


// Command descriptions start on page 83 (9341) / 141 (9488) of the datasheet

#define ILI_NOP         0x00    // -  No-op
#define ILI_SWRESET     0x01    // -  Software reset
#define ILI_RDDID       0x04    // -  Read display identification information (3)
#define ILI_RDDST       0x09    // -  Read Display Status (2)

#define ILI_SLPIN       0x10    // -  Enter Sleep Mode
#define ILI_SLPOUT      0x11    // -  Sleep Out
#define ILI_PTLON       0x12    // -  Partial Mode ON
#define ILI_NORON       0x13    // -  Normal Display Mode ON

#define ILI_RDMODE      0x0A    // -  Read Display Power Mode (1)
#define ILI_RDMADCTL    0x0B    // -  Read Display MADCTL (1)
#define ILI_RDPIXFMT    0x0C    // -  Read Display Pixel Format (1)
#define ILI_RDIMGFMT    0x0D    // -  Read Display Image Format (1)
#define ILI_RDSIGMODE   0x0E    // -  Read Display Signal Mode (1)
#define ILI_RDSELFDIAG  0x0F    // -  Read Display Self-Diagnostic Result (1)
#define ILI_RDID1       0xDA    // -  Read Display Manufacturer ID (1)
#define ILI_RDID2       0xDB    // -  Read Display Version ID (1)
#define ILI_RDID3       0xDC    // -  Read Display Module/Driver ID (1)
#define ILI_RDID4       0xD3    // -  Read Display IC (Ver, Model-A, Model-B) (3)

#define ILI_INVOFF      0x20    // -  Display Inversion OFF
#define ILI_INVON       0x21    // -  Display Inversion ON
#define ILI_GAMMASET    0x26    // -  Gamma Set
#define ILI_DISPOFF     0x28    // -  Display OFF
#define ILI_DISPON      0x29    // -  Display ON

#define ILI_CASET       0x2A    // -  Column Address Set
#define ILI_PASET       0x2B    // -  Page Address Set
#define ILI_RAMWR       0x2C    // -  Memory Write
#define ILI_CLRSET      0x2D    // -  Color Set
#define ILI_RAMRD       0x2E    // -  Memory Read

#define ILI_PTLAR       0x30    // -  Partial Area
#define ILI_VSCRDEF     0x33    // -  Vertical Scrolling Definition
#define ILI_MADCTL      0x36    // -  Memory Access Control
#define ILI_VSCRSADD    0x37    // -  Vertical Scrolling Start Address
#define ILI_PIXFMT      0x3A    // -  COLMOD: Pixel Format Set

#define ILI_FRMCTL1     0xB1    // -  Frame Rate Control (In Normal Mode/Full Colors)
#define ILI_FRMCTL2     0xB2    // -  Frame Rate Control (In Idle Mode/8 colors)
#define ILI_FRMCTL3     0xB3    // -  Frame Rate control (In Partial Mode/Full Colors)
#define ILI_INVCTL      0xB4    // -  Display Inversion Control
#define ILI_DFUNCTL     0xB6    // -  Display Function Control

#define ILI_PWCTL1      0xC0    // -  Power Control 1
#define ILI_PWCTL2      0xC1    // -  Power Control 2
#define ILI_PWCTL3      0xC2    // -  Power Control 3
#define ILI_PWCTL4      0xC3    // -  Power Control 4
#define ILI_PWCTL5      0xC4    // -  Power Control 5
#define ILI_VMCTL1      0xC5    // -  VCOM Control 1
#define ILI_VMCTL2      0xC7    // -  VCOM Control 2

#define ILI_GMCTLP1     0xE0    // -  Positive Gamma Correction
#define ILI_GMCTLN1     0xE1    // -  Negative Gamma Correction
#define ILI_PWCTL6      0xFC    // -  Power Control

/**
 * @brief ILI Controller type.
 * @ingroup display
 */
typedef enum _ili_controller_type {
    ILI_CONTROLLER_NONE = 0,
    ILI_CONTROLLER_9341 = 9341,
    ILI_CONTROLLER_9488 = 9488,
} ili_controller_type;

/**
 * @brief ILI Display information.
 * @ingroup display
 */
typedef struct _ili_disp_info_ {
    // ID (CMD 0x04)
    uint8_t lcd_mfg_id;
    uint8_t lcd_version;
    uint8_t lcd_id;
    // Display Status (CMD 0x09)
    uint8_t status1;
    uint8_t status2;
    uint8_t status3;
    uint8_t status4;
    // Power Mode (CMD 0x0A)
    uint8_t pwr_mode;
    // MADCTL (CMD 0x0B)
    uint8_t madctl;
    // PIXEL Format (CMD 0x0C)
    uint8_t pixelfmt;
    // Image Format (CMD 0x0D)
    uint8_t imagefmt;
    // Signal Mode (CMD 0x0E)
    uint8_t signal_mode;
    // Self-Diagnostic Result (CMD 0x0F)
    uint8_t selftest;
    // ID1 (MFG CMD 0xDA)
    uint8_t lcd_id1_mfg;
    // ID2 (Version CMD 0xDB)
    uint8_t lcd_id2_ver;
    // ID3 (Driver CMD 0xDB)
    uint8_t lcd_id3_drv;
    // ID4 (IC Ver,Model-l,Model-2 CMD 0xD3)
    uint8_t lcd_id4_ic_ver;
    uint8_t lcd_id4_ic_model1;
    uint8_t lcd_id4_ic_model2;
} ili_disp_info_t;

/**
 * @brief Send a command byte to the controller.
 * @ingroup display
 *
 * Sends a single byte command to the controller.
 * Care should be taken to avoid putting the controller into
 * an invalid/unknown state.
 *
 * @see ILI9341 datasheet section 8 and ILI9488 section 5 for command descriptions.
 *
 * @param cmd The command byte to send (@see ILI_xxx defines)
 */
extern void ili_send_command(uint8_t cmd);

/**
 * @brief Send a command byte and argument data to the controller.
 * @ingroup display
 *
 * Sends a command and additional argument data to the controller.
 * Care should be taken to avoid putting the controller into
 * an invalid/unknown state.

 * @see ILI9341 datasheet section 8 and ILI9488 section 5 for command descriptions.
 *
 * @param cmd The command byte to send (@see ILI_xxx defines)
 * @param data A pointer to a byte buffer of argument data
 * @param count The number of data bytes to send from the buffer
 */
extern void ili_send_command_wd(uint8_t cmd, uint8_t* data, size_t count);

/**
 * @brief Display all of the colors in the palette
 * @ingroup display
 *
 * Run through all of the colors. First do red, then green, then blue.
 * Second, run through all 64k colors, going from 0 to 65,535.
 */
extern void ili_colors_show();

/**
 * @brief Get a pointer to a buffer large enough to hold
 * one scan line for the ILI display.
 * @ingroup display
 *
 * This can be used to put RGB data into to be written to the
 * screen. The `ili_line_paint` can be called to put the
 * line on the screen.
 *
 * The buffer holds `ILI_WIDTH` rgb_t values.
 */
extern rgb16_t* ili_get_line_buf();

/**
 * @brief Get information about the display hardare & configuration.
 * @ingroup display
*/
extern ili_disp_info_t* ili_info(void);

/**
 * @brief Initialize the display.
 * @ingroup display
*/
extern ili_controller_type ili_module_init(void);

/**
 * @brief Paint a buffer of rgb16_t values to one horizontal line
 * of the screen. The buffer passed in must be at least `ILI_WIDTH`
 * rgb16_t values in size.
 * @ingroup display
 *
 * @param line 0-based line number to paint (must be less than `ILI_WIDTH`).
 * @param buf pointer to a rgb16_t buffer of data (must be at least 'ILI_WIDTH`)
 */
extern void ili_line_paint(uint16_t line, rgb16_t* buf);

/**
 * @brief Clear the entire screen.
 * @ingroup display
 *
 * @param force True to force a write to the screen. Otherwise, the screen is writen
 *              to only if the screen is thought to be 'dirty'.
 */
extern void ili_screen_clr(rgb16_t color, bool force);

/**
 * @brief The height of the display screen (pixel lines).
 * @ingroup display
 *
 * @return uint16_t Pixel lines
 */
extern uint16_t ili_screen_height();

/**
 * @brief Turn the screen (display) on/off.
 * @ingroup display
 *
 * @param on True to turn on, false to turn off
 */
extern void ili_screen_on(bool on);

/**
 * @brief Paint the screen with the RGB-12 contents of a buffer.
 * @ingroup display
 *
 * Uses the buffer of RGB data to paint the screen into the screen window.
 * Set the screen window using `ili_window_set_area`.
 *
 * @param data RGB-12 pixel data buffer (1 rgb value for each pixel to paint)
 * @param pixels Number of pixels (size of the data buffer in rgb_t's)
 */
extern void ili_screen_paint(const rgb16_t* rgb_pixel_data, uint16_t pixels);

/**
 * @brief The width of the display screen (pixel columns).
 * @ingroup display
 *
 * @return uint16_t Pixal columns
 */
extern uint16_t ili_screen_width();

/**
 * @brief Exit scroll mode to normal mode.
 * @ingroup display
 *
 * This puts the screen back into normal display mode (no scroll area) and
 * sets the window to full screen.
 */
extern void ili_scroll_exit(void);

/**
 * @brief Set the scroll area. It is between the top fixed area and the
 * bottom fixed area.
 * @ingroup display
 *
 * NOTE: Hardware scrolling only works when the display is in portrait mode
 * (MADCTL MV (bit 5) = 0)
 *
 * @see ili_scroll_set_start
 *
 * @param top_fixed_lines Number of fixed lines at the top of the screen
 * @param bottom_fixed_lines Number of fixed lines at the bottom of the screen
 */
extern void ili_scroll_set_area(uint16_t top_fixed_lines, uint16_t bottom_fixed_lines);

/**
 * @brief Set the frame memory start line for the scroll area.
 * @ingroup display
 *
 * ILI9341:
 * The ILI9341 datasheet doesn't do a great job of describing the scroll functionality
 * and this value, as there is only an example for when the top and bottom fixed areas
 * are 0. For that case, this value needs to range from 0 to 319 (the display height
 * in portrait mode (MADCTL-MV (bit 5) = 0) a requirement of the ILI9341)),
 * and it defines the frame memory line that is displayed at the top of the
 * display (line 0 of the display).
 *
 * ILI9488:
 * ZZZ
 *
 * When the top, and/or bottom fixed areas are non-zero, the scroll start needs to be
 * greater than the top fixed line and less than the bottom fixed line. If it is set
 * within either the top or bottom fixed areas it is the same as being set at the
 * minimum or maximum limit.
 *
 * @see ili_scroll_set_area
 *
 * @param line The line within the frame memory to display at the top of the scroll area.
 */
extern void ili_scroll_set_start(uint16_t line);

/**
 * @brief Set the screen update window and position the start at x,y.
 * @ingroup display
 *
 * Sets the update window area on the screen. This is the area that RGB data
 * will be updated into using `ili_screen_paint`.
 */
extern void ili_window_set_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * @brief Set the screen update window to the full screen, and position the
 * start at 0,0.
 * @ingroup display
 *
 * Sets the update window to the full screen and positions the start at 0,0.
*/
extern void ili_window_set_fullscreen(void);


#ifdef __cplusplus
    }
#endif
#endif // _ILI_LCD_SPI_H_