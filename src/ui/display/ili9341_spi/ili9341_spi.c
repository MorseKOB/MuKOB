/**
 * ILI9341 TFT LCD functionaly interface through SPI
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * Using ILI9341 4-Line Serial Interface II (see datasheet pg 63)
 * 64k Color (16bit) Mode
 */
#include "pico/stdlib.h"

#include "system_defs.h"
#include "ili9341_spi.h"
#include "spi_ops.h"

#include <stdlib.h>
#include <string.h>

static void _write_area(const rgb16_t *rgb_pixel_data, uint16_t pixels);
static void _set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
static void _set_window_fullscreen(void);

static rgb16_t _ili9341_line_buf[ILI9341_WIDTH];

// Storage for last used screen window location
static uint16_t _old_x1 = 0xffff, _old_x2 = 0xffff;
static uint16_t _old_y1 = 0xffff, _old_y2 = 0xffff;

/** @brief Flag to track if the screen has been written to since we know it was cleared. */
static bool _screen_dirty = true; // start out assuming dirty

static const uint8_t _ili9341_initcmd[] = {
    // CMD, ARGC, ARGD...
    // (if ARGC bit 8 set, delay after sending command)
    ILI9341_PWCTL1  , 1, 0x23,              // Grayscale voltage level (GVDD): 3.8v
    ILI9341_PWCTL2  , 1, 0x13,              // AVDD=VCIx2 VGH=VCIx6 VGL=-VCIx3
    ILI9341_VMCTL1  , 2, 0x3e, 0x28,        // VCOM Voltage: VCH=4.25v VCL:-1.5v
    ILI9341_VMCTL2  , 1, 0x86,              // VCOM Offset: Enabled, VMH-58, VML-58
    ILI9341_MACTL   , 1, 0x98,  //0xE8,              // Memory Access Control: YX=Invert, Landscape, BGR, Vnorm, Hnorm
    ILI9341_VSCRSADD, 2, 0x00, 0x00,        // Vertical Scroll Start: 0x0000
    ILI9341_PIXFMT  , 1, 0x55,              // Pixel Format: 16 RGB 5,6,5 bits
    ILI9341_FRMCTL1 , 2, 0x00, 0x1B,        // Frame Ctl (Normal Mode): fosc/1, rate=70Hz
    ILI9341_DFUNCTL , 4, 0x08,              // Function Ctl: Non-Disp Inverval Scan, V63/V0, VCH/VCL,
                         0x82,              //  LCD=Normally White, ScanG=0-320, ScanS=0-720, ScanMode=T-B, Int=5fms
                         0x27,              //  LCD Interval Drive Lines=192
                         0x00,              //  PCDIV (clock divider factor)=0
    ILI9341_GAMMASET , 1, 0x01,             // Gamma curve 1 selected
    ILI9341_GMCTLP1 , 15, 0x0F, 0x31,       // Set Gamma Positive correction
        0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09,
        0x00,
    ILI9341_GMCTLN1 , 15, 0x00, 0x0E,       // Set Gamma Negative correction
        0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36,
        0x0F,
    ILI9341_SLPOUT  , 0x80,                 // Exit Sleep
    ILI9341_DISPON  , 0x80,                 // Display on
    0x00                                    // End of list
};

static ili9341_disp_info_t _ili9341_disp_info;

/**
 * Set the chip select for the display.
 *
 */
static void _cs(bool sel) {
    if (sel) {
        gpio_put(SPI_CS_DISPLAY, SPI_CS_ENABLE);
    }
    else {
        gpio_put(SPI_CS_DISPLAY, SPI_CS_DISABLE);
    }
}

/**
 * Set the data/command select for the display.
 *
 */
static void _command_mode(bool cmd) {
    if (cmd) {
        gpio_put(SPI_DC_DISPLAY, DISPLAY_DC_CMD);
    }
    else {
        gpio_put(SPI_DC_DISPLAY, DISPLAY_DC_DATA);
    }
}

static void _op_begin() {
    spi_display_begin();
    _cs(true);
}

static void _op_end() {
    _cs(false);
    spi_display_end();
}

/**
 * @brief Read register values from the controller.
 * MUST BE WITHIN `_op_begin` and `_op_end`!!!
 *
 * Send a command indicating what to read and then read a
 * number of data bytes.
*/
static void _read_controller_values(uint8_t cmd, uint8_t* data, size_t count) {
    _command_mode(true);
    spi_display_write(&cmd, 1);

    _command_mode(false);
    spi_display_read(0xFF, data, count);
}

/**
 * @brief Send a single command byte to the controller.
 * MUST BE WITHIN `_op_begin` and `_op_end`!!!
*/
static void _send_command(uint8_t cmd) {
    _command_mode(true);
    spi_display_write(&cmd, 1);
    _command_mode(false);
}

/**
 * @brief Send a command and 0-n data bytes of data to the controller.
 * MUST BE AFTER `_op_begin`!!! An '_op_end` should follow after the data.
*/
static void _send_command_wd(uint8_t cmd, const uint8_t* data, size_t count) {
    _send_command(cmd);
    spi_display_write(data, count);
}

/** @brief Set window. MUST BE CALLED WITHIN `_op_begin` and `_op_end`!!! */
static void _set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t x2 = (x + w - 1), y2 = (y + h - 1);
    uint16_t words[2];
    if (x != _old_x1 || x2 != _old_x2) {
        _send_command(ILI9341_CASET); // Column address set
        words[0] = x;
        words[1] = x2;
        spi_display_write16(words, 2);
        _old_x1 = x;
        _old_x2 = x2;
    }
    if (y != _old_y1 || y2 != _old_y2) {
        _send_command(ILI9341_PASET); // Page address set
        words[0] = y;
        words[1] = y2;
        spi_display_write16(words, 2);
        _old_y1 = y;
        _old_y2 = y2;
    }
    _send_command(ILI9341_RAMWR); // Set it up to write to RAM
}

/** @brief Set window to fullscreen. MUST BE CALLED WITHIN `_op_begin` and `_op_end`!!! */
static void _set_window_fullscreen(void) {
    _set_window(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
}

/**
 * @brief Paint a buffer onto the screen. MUST BE CALLED WITHIN AN OPERATION!
*/
static void _write_area(const rgb16_t* rgb_pixel_data, uint16_t pixels) {
    spi_display_write16(rgb_pixel_data, pixels);
}

/**
 * Show all of the colors.
 */
void ili9341_colors_show() {
    ili9341_screen_clr(false);
    _op_begin();
    {
        // Do each color 4x4
        _set_window(0, 0, 32 * 4, 4);
        // Reds...
        for (int row = 0; row < 4; row++) {
            for (int r = 0; r < 32; r++) {
                rgb16_t red = r << 11;
                for (int col = 0; col < 4; col++) {
                    spi_display_write16(&red, 1);
                }
            }
        }
        // Greens...
        _set_window(0, 4, 32 * 4, 4);
        for (int row = 0; row < 4; row++) {
            for (int g = 0; g < 64; g++) {
                rgb16_t grn = g << 5;
                for (int col = 0; col < 2; col++) {
                    spi_display_write16(&grn, 1);
                }
            }
        }
        // Blues...
        _set_window(0, 8, 32 * 4, 4);
        for (int row = 0; row < 4; row++) {
            for (int b = 0; b < 32; b++) {
                rgb16_t blu = b;
                for (int col = 0; col < 4; col++) {
                    spi_display_write16(&blu, 1);
                }
            }
        }
        // Colors 0 - 0xFFFF
        _set_window(0, 12, 320, 228);
        for (rgb16_t i = 0; i < 0xFFFF; i++) {
            spi_display_write16(&i, 1);
        }
    }
    _op_end();
    _screen_dirty = true;
}

void ili9341_command(uint8_t cmd){
    _op_begin();
    {
        _send_command(cmd);
    }
    _op_end();
}

void ili9341_command_wd(uint8_t cmd, uint8_t* data, size_t count) {
    _op_begin();
    {
        _send_command_wd(cmd, data, count);
    }
    _op_end();
}

rgb16_t* ili9341_get_line_buf() {
    return (_ili9341_line_buf);
}

void ili9341_line_paint(uint16_t line, rgb16_t *buf) {
    if (line >= ILI9341_HEIGHT) {
        return;
    }
    _op_begin();
    {
        _set_window(0, line, ILI9341_WIDTH, 1);
        _write_area(buf, ILI9341_WIDTH);
    }
    _op_end();
}

void ili9341_screen_clr(bool force) {
    if (force || _screen_dirty) {
        memset(_ili9341_line_buf, 0, sizeof(_ili9341_line_buf));
        _op_begin();
        {
            _set_window_fullscreen();
            for (int i = 0; i < ILI9341_HEIGHT; i++) {
                _write_area(_ili9341_line_buf, ILI9341_WIDTH);
            }
        }
        _op_end();
        _screen_dirty = false;
    }
    else {
        // The screen wasn't dirty, so we didn't clear it,
        // but people expect a call to clear to set the
        // window to the full screen. Check and set if needed.
        if (_old_x1 != 0 || _old_y1 != 0 || _old_x2 != (ILI9341_WIDTH - 1) || _old_y2 != (ILI9341_HEIGHT - 1)) {
            ili9341_window_set_fullscreen();
        }
    }
}

void ili9341_screen_paint(const rgb16_t* rgb_pixel_data, uint16_t pixels) {
    _op_begin();
    {
        _write_area(rgb_pixel_data, pixels);
    }
    _op_end();
    _screen_dirty = true;
}

/**
 * Read information about the display status and the current configuration.
*/
ili9341_disp_info_t* ili9341_spi_info(void) {
    // The info/status reads require that a command be sent,
    // then a 'dummy' byte read, then one or more data reads.
    //
    // The largest read is a 'dummy' + 4 bytes, so use a 5 byte array to read into.
    uint8_t data[5];

    _op_begin();
    {
        // ID
        _read_controller_values(ILI9341_RDDID, data, 4);
        _ili9341_disp_info.lcd_mfg_id = data[0];
        _ili9341_disp_info.lcd_version = data[1];
        _ili9341_disp_info.lcd_id = data[2];
        // Display Status
        _read_controller_values(ILI9341_RDDST, data, 5);
        _ili9341_disp_info.status1 = data[0];
        _ili9341_disp_info.status2 = data[1];
        _ili9341_disp_info.status3 = data[2];
        _ili9341_disp_info.status4 = data[3];
        // Power Mode
        _read_controller_values(ILI9341_RDMODE, data, 2);
        _ili9341_disp_info.pwr_mode = data[0];
        // MADCTL
        _read_controller_values(ILI9341_RDMADCTL, data, 2);
        _ili9341_disp_info.madctl = data[0];
        // Pixel Format
        _read_controller_values(ILI9341_RDPIXFMT, data, 2);
        _ili9341_disp_info.pixelfmt = data[0];
        // Image Format
        _read_controller_values(ILI9341_RDIMGFMT, data, 2);
        _ili9341_disp_info.imagefmt = data[0];
        // Signal Mode
        _read_controller_values(ILI9341_RDSIGMODE, data, 2);
        _ili9341_disp_info.signal_mode = data[0];
        // Selftest result
        _read_controller_values(ILI9341_RDSELFDIAG, data, 2);
        _ili9341_disp_info.selftest = data[0];
        // ID 1 (Manufacturer)
        _read_controller_values(ILI9341_RDID1, data, 2);
        _ili9341_disp_info.lcd_id1_mfg = data[0];
        // ID 2 (Version)
        _read_controller_values(ILI9341_RDID2, data, 2);
        _ili9341_disp_info.lcd_id2_ver = data[0];
        // ID 3 (Driver)
        _read_controller_values(ILI9341_RDID3, data, 2);
        _ili9341_disp_info.lcd_id3_drv = data[0];
        // ID 4 (IC)
        _read_controller_values(ILI9341_RDID4, data, 4);
        _ili9341_disp_info.lcd_id4_icv  = data[0];
        _ili9341_disp_info.lcd_id4_icm1 = data[1];
        _ili9341_disp_info.lcd_id4_icm2 = data[2];
    }
    _op_end();

    return (&_ili9341_disp_info);
}

void ili9341_spi_init(void) {
    // Take reset low, then high
    gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_OFF);
    sleep_ms(20);
    gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_ON);
    sleep_ms(20);
    gpio_put(DISPLAY_RESET_OUT, DISPLAY_HW_RESET_OFF);
    sleep_ms(100);  // Wait for it to come up
    gpio_put(DISPLAY_BACKLIGHT_OUT, DISPLAY_BACKLIGHT_ON);

    uint8_t cmd, x, numArgs;
    const uint8_t* init_cmd_data = _ili9341_initcmd;
    _op_begin();
    {
        while ((cmd = *(init_cmd_data++)) > 0) {
            x = *(init_cmd_data++);
            numArgs = x & 0x7F;
            _send_command_wd(cmd, init_cmd_data, numArgs);
            init_cmd_data += numArgs;
            if (x & 0x80)
                sleep_ms(150);
        }
    }
    _op_end();
}

void ili9341_scroll_exit(void) {
    _op_begin();
    {
        _send_command(ILI9341_DISPOFF);
        _send_command(ILI9341_NORON);
        _send_command(ILI9341_DISPON);
        _set_window_fullscreen();
    }
    _op_end();
}

void ili9341_scroll_set_area(uint16_t top_fixed_lines, uint16_t bottom_fixed_lines) {
    uint16_t words[3];
    _op_begin();
    {
        _send_command(ILI9341_VSCRDEF); // Vertical Scrolling Definition
        words[0] = top_fixed_lines;
        words[1] = ILI9341_HEIGHT - (top_fixed_lines + bottom_fixed_lines);
        words[2] = bottom_fixed_lines;
        spi_display_write16(words, 3);
        _send_command(ILI9341_VSCRSADD);
        uint16_t row = top_fixed_lines;
        spi_display_write16(&row, 1);
        // Set window within the scroll area
    }
    _op_end();
}

void ili9341_scroll_set_start(uint16_t row){
    _op_begin();
    {
        _send_command(ILI9341_VSCRSADD);
        spi_display_write16(&row, 1);
    }
    _op_end();
}

void ili9341_window_set_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    _op_begin();
    {
        _set_window(x, y, w, h);
    }
    _op_end();
}

void ili9341_window_set_fullscreen(void) {
    _op_begin();
    {
        _set_window(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
    }
    _op_end();
}
