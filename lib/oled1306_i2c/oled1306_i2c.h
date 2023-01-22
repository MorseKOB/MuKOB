/**
 * Copyright 2023 AESilky
 * Portions copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 * 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef OLED1306_I2C_H
#define OLED1306_I2C_H
#ifdef __cplusplus
 extern "C" {
#endif

/**
 * Some portions from Raspberry Pi Pico example code to talk to an SSD1306-based OLED display
 * https://github.com/raspberrypi/pico-examples/tree/master/i2c/oled_i2c
 * 
 * (SSD1306 Datasheet: https://www.digikey.com/htmldatasheets/production/2047793/0/0/1/SSD1306.pdf)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "font.h"

// I2C values if not set outside...
#ifndef I2C_PORT
#define I2C_PORT i2c_default
#endif

// commands (see datasheet)
#define OLED_SET_CONTRAST _u(0x81)
#define OLED_SET_ENTIRE_ON _u(0xA4)
#define OLED_SET_NORM_INV _u(0xA6)
#define OLED_SET_DISP _u(0xAE)
#define OLED_SET_MEM_ADDR_MODE _u(0x20)
#define OLED_SET_COL_ADDR _u(0x21)
#define OLED_SET_PAGE_ADDR _u(0x22)
#define OLED_SET_DISP_START_LINE _u(0x40)
#define OLED_SET_SEG_REMAP _u(0xA0)
#define OLED_SET_MUX_RATIO _u(0xA8)
#define OLED_SET_COM_OUT_DIR _u(0xC0)
#define OLED_SET_DISP_OFFSET _u(0xD3)
#define OLED_SET_COM_PIN_CFG _u(0xDA)
#define OLED_SET_DISP_CLK_DIV _u(0xD5)
#define OLED_SET_PRECHARGE _u(0xD9)
#define OLED_SET_VCOM_DESEL _u(0xDB)
#define OLED_SET_CHARGE_PUMP _u(0x8D)
#define OLED_SET_HORIZ_SCROLL _u(0x26)
#define OLED_SET_SCROLL _u(0x2E)
//
#define OLED_ADDR _u(0x3C)
#define OLED_HEIGHT _u(64)
#define OLED_WIDTH _u(128)
#define OLED_PAGE_HEIGHT _u(8)
#define OLED_NUM_PAGES OLED_HEIGHT / OLED_PAGE_HEIGHT
#define OLED_BUF_LEN (OLED_NUM_PAGES * OLED_WIDTH)
//
#define OLED_WRITE_MODE _u(0xFE)
#define OLED_READ_MODE _u(0xFF)
//
// Text handling
#define CHAR_ROWS 7
#define CHAR_COLS 16
#define STATUS_ROW 0

struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;
    int buflen;
};

/*! \brief Memory area for the screen data pixel-bytes */
extern uint8_t display_buf[];

void fill(uint8_t buf[], uint8_t fill);

void fill_page(uint8_t *buf, uint8_t fill, uint8_t page);

void calc_render_area_buflen(struct render_area *area);

void oled_send_cmd(uint8_t cmd);

void oled_send_buf(uint8_t buf[], int buflen);

void oled_init();

void render(uint8_t *buf, struct render_area *area);

/*! \brief Display a string
 *  \ingroup oled1306_i2c
 *
 * Display a string of ASCII characters (plus some special characters)
 * The font characters are in a 8x9 matrix allowing for 7 lines of 16 characters
 * on the 128x64 dot display.
 * 
 * The first line includes an extra line of whitespace to allow it to be used
 * as a header/status bar.
 * 
 * \param row 0-6 With 0 being the top (status) line
 * \param col 0-127 Starting column
 * \param c character to display
 * 
 * Using a 9 pixel high font on an 64 pixel high screen with 8 pixel high pages, with the 
 * first row having an extra line of whitespace seperation from the next row (to create a 'status line')
 * gives us a memory layout as follows:
 * 
 * PxR | Page | Row
 * -----------------
 *   0 |    0 |  0
 *  .. |      |
 *  ----------|
 *   8 |    1 |
 *   9 |      |XXXXX
 *     |      |-----
 *  10 |      |  1
 *  .. |      |
 *  ----------|
 *  16 |    2 |
 *     |      |-----
 *  19 |      |  2
 *  .. |      |
 *  ----------|
 *  24 |    3 |
 *  .. |      |
 *     |      |-----
 *  28 |      |  3
 *  .. |      |
 *  ----------|
 *  32 |    4 |
 *  .. |      |
 *     |      |-----
 *  37 |      |  4
 *  .. |      |
 *  ----------|
 *  40 |    5 |
 *  .. |      |
 *     |      |-----
 *  46 |      |  5
 *  47 |      |
 *  ----------|
 *  48 |    6 |
 *     |      |-----
 *  49 |      |  6
 *  .. |      |
 *  ----------|
 *  56 |    7 |
 *  .. |      |
 *  63 |      |
 *  ----------------
 *  
 * Therefore; writing a character into row-col on the screen requires that it be merged in with 
 * existing buffer data from two pages using masks that depend on what row it is being written.
 * The masks are:
 * Row 0 0-7: P0:00
 *       8  : P1:3F
 * 
 * Row 1 0-7: P1:C0
 *       8  : P2:7F
 * 
 * Row 2 0-3: P2:E0
 *       4-
 *  
 */
// void display_char(int row, int col, const char c) {
//     if (row < 0 || row > 6 || col < 0 || col > 127) {
//         debug_out("ERROR: Row or Col out of range - %d, %d\n", row, col);
//         return;
//     }

//     while (*pString != '\0')
//     {
//         // Find starting pixel (top-right)
//         if ((Xpoint + Font9->Width) > Paint.Width)
//         {
//             Xpoint = Xstart;
//             Ypoint += Font->Height;
//         }

//         // If the Y direction is full, reposition to(Xstart, Ystart)
//         if ((Ypoint + Font->Height) > Paint.Height)
//         {
//             Xpoint = Xstart;
//             Ypoint = Ystart;
//         }
//         Paint_DrawChar(Xpoint, Ypoint, *pString, Font, Color_Background, Color_Foreground);

//         // The next character of the address
//         pString++;

//         // The next word of the abscissa increases the font of the broadband
//         Xpoint += Font->Width;
//     }
// }

// /*! \brief Display a string
//  *  \ingroup oled1306_i2c
//  *
//  * Display a string of ASCII characters (plus some special characters)
//  * The font characters are in a 8x9 matrix allowing for 7 lines of 16 characters
//  * on the 128x64 dot display.
//  * 
//  * The first line includes an extra line of whitespace to allow it to be used
//  * as a header/status bar.
//  * 
//  * \param row 0-6 With 0 being the top (status) line
//  * \param col 0-127 Starting column
//  * \param pString Pinter to the first character of a null-terminated string
//  */
// void display_string(int row, int col, const char *pString) {
//     if (row < 0 || row > 6 || col < 0 || col > 127) {
//         debug_out("ERROR: Row or Col out of range - %d, %d\n", row, col);
//         return;
//     }

//     while (*pString != '\0')
//     {
//         // Find starting pixel (top-right)
//         if ((Xpoint + Font9->Width) > Paint.Width)
//         {
//             Xpoint = Xstart;
//             Ypoint += Font->Height;
//         }

//         // If the Y direction is full, reposition to(Xstart, Ystart)
//         if ((Ypoint + Font->Height) > Paint.Height)
//         {
//             Xpoint = Xstart;
//             Ypoint = Ystart;
//         }
//         Paint_DrawChar(Xpoint, Ypoint, *pString, Font, Color_Background, Color_Foreground);

//         // The next character of the address
//         pString++;

//         // The next word of the abscissa increases the font of the broadband
//         Xpoint += Font->Width;
//     }
// }

/*! \brief Scroll display horizontally
 *  \ingroup oled1306_i2c
 *
 *  Scroll the full display from right to left.
 */
void scroll_horz(void);

// ################################################################################################
// print_xxx convenience methods for printing out a buffer to be rendered
// mostly useful for debugging images, patterns, etc

void print_buf_page(uint8_t buf[], uint8_t page);

void print_buf_pages(uint8_t buf[]);

void print_buf_area(uint8_t *buf, struct render_area *area);

#ifdef __cplusplus
}
#endif
#endif // OLED1306_I2C_H
