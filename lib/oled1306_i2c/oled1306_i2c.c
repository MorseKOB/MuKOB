/**
 * Copyright 2023 AESilky
 * Portions copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 * 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * Some portions from Raspberry Pi Pico example code to talk to an SSD1306-based OLED display
 * https://github.com/raspberrypi/pico-examples/tree/master/i2c/oled_i2c
 * 
 * (SSD1306 Datasheet: https://www.digikey.com/htmldatasheets/production/2047793/0/0/1/SSD1306.pdf)
 */
#include "oled1306_i2c.h"

/*! \brief Memory area for the screen data pixel-bytes */
uint8_t display_buf[OLED_BUF_LEN];
/*! \brief Memory area for a screen of text (characters) */
char text_screen[CHAR_ROWS * CHAR_COLS];

/*! \brief Memory area for the screen data pixel-bytes */
struct render_area display_area = {start_col: 0, end_col : OLED_WIDTH - 1, start_page : 0, end_page : OLED_NUM_PAGES - 1};

void fill(uint8_t buf[], uint8_t fill) {
    // fill entire buffer with the same byte
    for (int i = 0; i < OLED_BUF_LEN; i++) {
        buf[i] = fill;
    }
};

void fill_page(uint8_t *buf, uint8_t fill, uint8_t page) {
    // fill entire page with the same byte
    memset(buf + (page * OLED_WIDTH), fill, OLED_WIDTH);
};

void calc_render_area_buflen(struct render_area *area) {
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

void oled_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command

    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(I2C_PORT, (OLED_ADDR & OLED_WRITE_MODE), buf, 2, false);
}

void oled_send_buf(uint8_t buf[], int buflen) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    // TODO find a more memory-efficient way to do this..
    // maybe break the data transfer into pages?
    uint8_t *temp_buf = malloc(buflen + 1);

    for (int i = 1; i < buflen + 1; i++) {
        temp_buf[i] = buf[i - 1];
    }
    // Co = 0, D/C = 1 => the driver expects data to be written to RAM
    temp_buf[0] = 0x40;
    i2c_write_blocking(I2C_PORT, (OLED_ADDR & OLED_WRITE_MODE), temp_buf, buflen + 1, false);

    free(temp_buf);
}

void oled_init() {
    // some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like

    // some configuration values are recommended by the board manufacturer

    oled_send_cmd(OLED_SET_DISP | 0x00);            // set display off

    /* memory mapping */
    oled_send_cmd(OLED_SET_MEM_ADDR_MODE);          // set memory address mode
    oled_send_cmd(0x00);                            // horizontal addressing mode

    /* resolution and layout */
    oled_send_cmd(OLED_SET_DISP_START_LINE);        // set display start line to 0

    oled_send_cmd(OLED_SET_MUX_RATIO);              // set multiplex ratio
    oled_send_cmd(OLED_HEIGHT - 1);                 // our display is 64 pixels high

    oled_send_cmd(OLED_SET_SEG_REMAP | 0x01);       // set segment re-map (A0/A1)
                                                    // A0 = column address 0 is mapped to SEG0
                                                    // A1 = column address 127 is mapped to SEG0

    oled_send_cmd(OLED_SET_COM_OUT_DIR | 0x08);     // set COM (common) output scan direction (C0/C8)
                                                    // C0 = scan from top down, COM0 to COM[N-1]
                                                    // C8 = scan from bottom up, COM[N-1] to COM0

    oled_send_cmd(OLED_SET_DISP_OFFSET);            // set display offset
    oled_send_cmd(0x00);                            // no offset

    oled_send_cmd(OLED_SET_COM_PIN_CFG);            // set COM (common) pins hardware configuration
    oled_send_cmd(0x10 | 0x02);                     // 00xx0010 
                                                    // xx:00 = sequential rows, no left/right remap
                                                    // xx:01 = interleave rows, no left/right remap
                                                    // xx:10 = sequential rows, left/right remap
                                                    // xx:11 = interleave rows, left/right remap

    /* timing and driving scheme */
    oled_send_cmd(OLED_SET_DISP_CLK_DIV);           // set display clock divide ratio
    oled_send_cmd(0x80);                            // div ratio of 1, standard freq

    oled_send_cmd(OLED_SET_PRECHARGE);              // set pre-charge period
    oled_send_cmd(0xF1);                            // Vcc internally generated on our board

    oled_send_cmd(OLED_SET_VCOM_DESEL);             // set VCOMH deselect level
    oled_send_cmd(0x30);                            // 0.83xVcc

    /* display */
    oled_send_cmd(OLED_SET_CONTRAST);               // set contrast control (0-255)
    oled_send_cmd(0xA0);

    oled_send_cmd(OLED_SET_ENTIRE_ON);              // set entire display on to follow RAM content

    oled_send_cmd(OLED_SET_NORM_INV);               // set normal (not inverted) display

    oled_send_cmd(OLED_SET_CHARGE_PUMP);            // set charge pump
    oled_send_cmd(0x14);                            // Vcc internally generated on our board

    oled_send_cmd(OLED_SET_SCROLL | 0x00);          // deactivate horizontal scrolling if set
        // this is necessary as memory writes will corrupt if scrolling is enabled

    oled_send_cmd(OLED_SET_DISP | 0x01);            // turn display on

    // initialize render area for entire display (128 pixels by 8 pages)
    calc_render_area_buflen(&display_area);

    // fill the character buffer with NULL's
    for (int i = 0; i < (CHAR_ROWS * CHAR_COLS); i++) {
        text_screen[i] = 0x00;
    }
    // zero the entire display
    fill(display_buf, 0x00);
    render(display_buf, &display_area);

    // intro sequence: flash the screen twice
    for (int i = 0; i < 2; i++) {
        oled_send_cmd(0xA5); // ignore RAM, all pixels on
        sleep_ms(200);
        oled_send_cmd(0xA4); // go back to following RAM
        sleep_ms(200);
    }
}

void render(uint8_t *buf, struct render_area *area) {
    // update a portion of the display with a render area
    oled_send_cmd(OLED_SET_COL_ADDR);
    oled_send_cmd(area->start_col);
    oled_send_cmd(area->end_col);

    oled_send_cmd(OLED_SET_PAGE_ADDR);
    oled_send_cmd(area->start_page);
    oled_send_cmd(area->end_page);

    oled_send_buf(buf, area->buflen);
}

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
void scroll_horz(void) {
    oled_send_cmd(OLED_SET_HORIZ_SCROLL | 0x00);
    oled_send_cmd(0x00); // dummy byte
    oled_send_cmd(0x00); // start page 0
    oled_send_cmd(0x00); // time interval
    oled_send_cmd(0x07); // end page 7
    oled_send_cmd(0x00); // dummy byte
    oled_send_cmd(0xFF); // dummy byte

    oled_send_cmd(OLED_SET_SCROLL | 0x01);
}

// ################################################################################################
// print_xxx convenience methods for printing out a buffer to be rendered
// mostly useful for debugging images, patterns, etc

void print_buf_page(uint8_t buf[], uint8_t page) {
    // prints one page of a full length (128x4) buffer
    for (int j = 0; j < OLED_PAGE_HEIGHT; j++) {
        for (int k = 0; k < OLED_WIDTH; k++) {
            printf("%u", (buf[page * OLED_WIDTH + k] >> j) & 0x01);
        }
        printf("\n");
    }
}

void print_buf_pages(uint8_t buf[]) {
    // prints all pages of a full length buffer
    for (int i = 0; i < OLED_NUM_PAGES; i++) {
        printf("--page %d--\n", i);
        print_buf_page(buf, i);
    }
}

void print_buf_area(uint8_t *buf, struct render_area *area) {
    // print a render area of generic size
    int area_width = area->end_col - area->start_col + 1;
    int area_height = area->end_page - area->start_page + 1; // in pages, not pixels
    for (int i = 0; i < area_height; i++) {
        for (int j = 0; j < OLED_PAGE_HEIGHT; j++) {
            for (int k = 0; k < area_width; k++) {
                printf("%u", (buf[i * area_width + k] >> j) & 0x01);
            }
            printf("\n");
        }
    }
}
