/**
 * ILI9341 LCD functionaly interface through SPI
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * Using ILI9341 4-Line Serial Interface II (see datasheet pg 63)
 * 262k Color (18bit) Mode - Easier to construct & send the data
 *  
 */
#include "pico/stdlib.h"

#include "system_defs.h"
#include "ili9341_spi.h"
#include "spi_ops.h"

#include <stdlib.h>
#include <string.h>

extern void _ili9341_write_area(const rgb_t *rgb_pixel_data, uint16_t pixels);
extern void _ili9341_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
extern void _ili9341_set_window_fullscreen(void);

static rgb_t __ili9341_line_buf[ILI9341_WIDTH];

// Storage for last used screen window location
static uint16_t __old_x1 = 0xffff, __old_x2 = 0xffff;
static uint16_t __old_y1 = 0xffff, __old_y2 = 0xffff;

/** \brief Flag to track if the screen has been written to since we know it was cleared. */
static bool __screen_dirty = true; // start out assuming dirty

static const uint8_t ili9341_initcmd[] = {
    // CMD, ARGC, ARGD...
    // (if ARGC bit 8 set, delay after sending command)
    ILI9341_PWCTL1  , 1, 0x23,              // Grayscale voltage level (GVDD): 3.8v
    ILI9341_PWCTL2  , 1, 0x13,              // AVDD=VCIx2 VGH=VCIx6 VGL=-VCIx3
    ILI9341_VMCTL1  , 2, 0x3e, 0x28,        // VCOM Voltage: VCH=4.25v VCL:-1.5v
    ILI9341_VMCTL2  , 1, 0x86,              // VCOM Offset: Enabled, VMH-58, VML-58
    ILI9341_MADCTL  , 1, 0xE8,              // Memory Access Control: YX=Invert, Landscape, BGR, Vnorm, Hnorm
    ILI9341_VSCRSADD, 2, 0x00, 0x00,        // Vertical Scroll Start: 0x0000  
    ILI9341_PIXFMT  , 1, 0x66,              // Pixel Format: 18 bits
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

static ili9341_disp_info_t __ili9341_disp_info;

/**
 * Set the chip select for the display.
 * 
 */
void _cs(bool sel) {
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
void _command_mode(bool cmd) {
    if (cmd) {
        gpio_put(SPI_DC_DISPLAY, ILI9341_DC_CMD);
    }
    else {
        gpio_put(SPI_DC_DISPLAY, ILI9341_DC_DATA);
    }
}

void _op_begin() {
    spi_begin();
    _cs(true);
}

void _op_end() {
    _cs(false);
    spi_end();
}

/**
 * \brief Read register values from the controller.
 * MUST BE WITHIN `_op_begin` and `_op_end`!!!
 * 
 * Send a command indicating what to read and then read a 
 * number of data bytes.
*/
void _read_controller_values(uint8_t cmd, uint8_t *data, size_t count) {
    _command_mode(true);
    spi_write(&cmd, 1);

    _command_mode(false);
    spi_read(0xFF, data, count);
}

/**
 * \brief Send a single command byte to the controller.
 * MUST BE WITHIN `_op_begin` and `_op_end`!!!
*/
void _send_command(uint8_t cmd) {
    _command_mode(true);
    spi_write(&cmd, 1);
    _command_mode(false);
}

/**
 * \brief Send a command and 0-n data bytes of data to the controller.
 * MUST BE AFTER `_op_begin`!!! An '_op_end` should follow after the data.
*/
void _send_command_wd(uint8_t cmd, const uint8_t *data, size_t count) {
    _send_command(cmd);
    spi_write(data, count);
}

rgb_t* ili9341_get_line_buf() {
    return (__ili9341_line_buf);
}

void ili9341_line_paint(uint16_t line, rgb_t *buf) {
    if (line >= ILI9341_HEIGHT) {
        return;
    }
    _op_begin();
    _ili9341_set_window(0, line, ILI9341_WIDTH, 1);
    _ili9341_write_area(buf, ILI9341_WIDTH);
    _op_end();
}

void ili9341_spi_init(void) {
    // Take reset low, then high
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_OFF);
    sleep_ms(20);
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_ON);
    sleep_ms(20);
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_OFF);
    sleep_ms(100);  // Wait for it to come up
    gpio_put(ILI9341_BACKLIGHT_OUT, ILI9341_BACKLIGHT_ON);

    uint8_t cmd, x, numArgs;
    const uint8_t *init_cmd_data = ili9341_initcmd;
    _op_begin();
    while ((cmd = *(init_cmd_data++)) > 0) {
        x = *(init_cmd_data++);
        numArgs = x & 0x7F;
        _send_command_wd(cmd, init_cmd_data, numArgs);
        init_cmd_data += numArgs;
        if (x & 0x80)
            sleep_ms(150);
    }
    _op_end();
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
    // ID
    _read_controller_values(ILI9341_RDDID, data, 4);
    __ili9341_disp_info.lcd_mfg_id = data[0];
    __ili9341_disp_info.lcd_version = data[1];
    __ili9341_disp_info.lcd_id = data[2];
    // Display Status
    _read_controller_values(ILI9341_RDDST, data, 5);
    __ili9341_disp_info.status1 = data[0];
    __ili9341_disp_info.status2 = data[1];
    __ili9341_disp_info.status3 = data[2];
    __ili9341_disp_info.status4 = data[3];
    // Power Mode
    _read_controller_values(ILI9341_RDMODE, data, 2);
    __ili9341_disp_info.pwr_mode = data[0];
    // MADCTL
    _read_controller_values(ILI9341_RDMADCTL, data, 2);
    __ili9341_disp_info.madctl = data[0];
    // Pixel Format
    _read_controller_values(ILI9341_RDPIXFMT, data, 2);
    __ili9341_disp_info.pixelfmt = data[0];
    // Image Format
    _read_controller_values(ILI9341_RDIMGFMT, data, 2);
    __ili9341_disp_info.imagefmt = data[0];
    // Signal Mode
    _read_controller_values(ILI9341_RDSIGMODE, data, 2);
    __ili9341_disp_info.signal_mode = data[0];
    // Selftest result
    _read_controller_values(ILI9341_RDSELFDIAG, data, 2);
    __ili9341_disp_info.selftest = data[0];
    // ID 1 (Manufacturer)
    _read_controller_values(ILI9341_RDID1, data, 2);
    __ili9341_disp_info.lcd_id1_mfg = data[0];
    // ID 2 (Version)
    _read_controller_values(ILI9341_RDID2, data, 2);
    __ili9341_disp_info.lcd_id2_ver = data[0];
    // ID 3 (Driver)
    _read_controller_values(ILI9341_RDID3, data, 2);
    __ili9341_disp_info.lcd_id3_drv = data[0];
    // ID 4 (IC)
    _read_controller_values(ILI9341_RDID4, data, 4);
    __ili9341_disp_info.lcd_id4_icv  = data[0];
    __ili9341_disp_info.lcd_id4_icm1 = data[1];
    __ili9341_disp_info.lcd_id4_icm2 = data[2];
    _op_end();

    return (&__ili9341_disp_info);
}

void ili9341_screen_clr(void) {
    if (__screen_dirty) {
        memset(__ili9341_line_buf, 0, sizeof(__ili9341_line_buf));
        _op_begin();
        _ili9341_set_window_fullscreen();
        for (int i = 0; i < ILI9341_HEIGHT; i++) {
            _ili9341_write_area(__ili9341_line_buf, ILI9341_WIDTH);
        }
        _op_end();
        __screen_dirty = false;
    }
    else {
        // The screen wasn't dirty, so we didn't clear it, 
        // but people expect a call to clear to set the 
        // window to the full screen. Check and set if needed.
        if (__old_x1 != 0 || __old_y1 != 0 || __old_x2 != (ILI9341_WIDTH - 1) || __old_y2 != (ILI9341_HEIGHT - 1)) {
            ili9341_set_window_fullscreen();
        }
    }
}

/**
 * \brief Paint a buffer onto the screen. MUST BE CALLED WITHIN AN OPERATION!
*/
void _ili9341_write_area(const rgb_t *rgb_pixel_data, uint16_t pixels) {
    spi_write((uint8_t*)rgb_pixel_data, pixels * sizeof(rgb_t));
}

/** \brief Set window. MUST BE CALLED WITHIN `_op_begin` and `_op_end`!!! */
void _ili9341_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t x2 = (x + w - 1), y2 = (y + h - 1);
    uint16_t words[2];
    if (x != __old_x1 || x2 != __old_x2) {
        _send_command(ILI9341_CASET); // Column address set
        words[0] = x;
        words[1] = x2;
        spi_write16(words, 2);
        __old_x1 = x;
        __old_x2 = x2;
    }
    if (y != __old_y1 || y2 != __old_y2) {
        _send_command(ILI9341_PASET); // Page address set
        words[0] = y;
        words[1] = y2;
        spi_write16(words, 2);
        __old_y1 = y;
        __old_y2 = y2;
    }
    _send_command(ILI9341_RAMWR); // Set it up to write to RAM
}

void ili9341_screen_paint(const rgb_t *rgb_pixel_data, uint16_t pixels) {
    _op_begin();
    _ili9341_write_area(rgb_pixel_data, pixels);
    _op_end();
    __screen_dirty = true;
}

void ili9341_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    _op_begin();
    _ili9341_set_window(x, y, w, h);
    _op_end();
}

/** \brief Set window to fullscreen. MUST BE CALLED WITHIN `_op_begin` and `_op_end`!!! */
void _ili9341_set_window_fullscreen(void) {
    _ili9341_set_window(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
}

void ili9341_set_window_fullscreen(void) {
    _op_begin();
    _ili9341_set_window(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
    _op_end();
}
