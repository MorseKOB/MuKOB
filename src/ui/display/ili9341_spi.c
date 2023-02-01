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
#include "system_defs.h"
#include "ili9341_spi.h"
#include "spi_ops.h"

static const uint8_t ili9341_initcmd[] = {
    // CMD, ARGC, ARGD...
    // (if ARGC bit 8 set, delay after sending command)
    //0xEF, 3, 0x03, 0x80, 0x02,
    //0xCF, 3, 0x00, 0xC1, 0x30,
    //0xED, 4, 0x64, 0x03, 0x12, 0x81,
    //0xE8, 3, 0x85, 0x00, 0x78,
    //0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    //0xF7, 1, 0x20,
    //0xEA, 2, 0x00, 0x00,
    ILI9341_PWCTL1  , 1, 0x23,              // Power control VRH[5:0]
    ILI9341_PWCTL2  , 1, 0x10,              // Power control SAP[2:0];BT[3:0]
    ILI9341_VMCTL1  , 2, 0x3e, 0x28,        // VCM control1 
    ILI9341_VMCTL2  , 1, 0x86,              // VCM control2
    ILI9341_MADCTL  , 1, 0x48,              // Memory Access Control
    ILI9341_VSCRSADD, 1, 0x00,              // Vertical scroll zero
    ILI9341_PIXFMT  , 1, 0x66,              // Pixel format 18 bits
    ILI9341_FRMCTL1 , 2, 0x00, 0x18,
    ILI9341_DFUNCTL , 3, 0x08, 0x82, 0x27,  // Display Function Control
    //0xF2, 1, 0x00,                        // 3Gamma Function Disable
    ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
    ILI9341_GMCTRP1 , 15, 0x0F, 0x31,       // Set Gamma
        0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 
        0x00,
    ILI9341_GMCTRN1 , 15, 0x00, 0x0E,       // Set Gamma
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
void display_cs(bool sel) {
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
void display_command_mode(bool cmd) {
    if (cmd) {
        gpio_put(SPI_DC_DISPLAY, ILI9341_DC_CMD);
    }
    else {
        gpio_put(SPI_DC_DISPLAY, ILI9341_DC_DATA);
    }
}

/**
 * Read data from the controller.
 * 
 * Send a command indicating what to read and then read a 
 * number of data bytes.
*/
void read_data(uint8_t cmd, uint8_t *data, size_t count) {
    spi_begin();
    display_cs(true);

    display_command_mode(true);
    spi_write(&cmd, 1);

    display_command_mode(false);
    spi_read(0xFF, data, count);
    
    display_cs(false);
    spi_end();
}

/**
 * Send a command and 0-n data bytes to the display.
*/
void send_command(uint8_t cmd, const uint8_t *data, size_t count) {
    spi_begin();
    display_cs(true);

    display_command_mode(true);
    spi_write(&cmd, 1);

    display_command_mode(false);
    spi_write(data, count);

    display_cs(false);
    spi_end();
}

void ili9341_spi_init(void) {
    // Take reset low, then high
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_OFF);
    sleep_ms(10);
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_ON);
    sleep_ms(10);
    gpio_put(ILI9341_RESET_OUT, ILI9341_HW_RESET_OFF);
    sleep_ms(10);
    gpio_put(ILI9341_BACKLIGHT_OUT, ILI9341_BACKLIGHT_ON);

    uint8_t cmd, x, numArgs;
    const uint8_t *init_cmd_data = ili9341_initcmd;
    while ((cmd = *(init_cmd_data++)) > 0) {
        x = *(init_cmd_data++);
        numArgs = x & 0x7F;
        send_command(cmd, init_cmd_data, numArgs);
        init_cmd_data += numArgs;
        if (x & 0x80)
            sleep_ms(150);
    }
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

    // ID
    read_data(ILI9341_RDDID, data, 4);
    __ili9341_disp_info.lcd_mfg_id = data[0];
    __ili9341_disp_info.lcd_version = data[1];
    __ili9341_disp_info.lcd_id = data[2];
    // Display Status
    read_data(ILI9341_RDDST, data, 5);
    __ili9341_disp_info.status1 = data[0];
    __ili9341_disp_info.status2 = data[1];
    __ili9341_disp_info.status3 = data[2];
    __ili9341_disp_info.status4 = data[3];
    // Power Mode
    read_data(ILI9341_RDMODE, data, 2);
    __ili9341_disp_info.pwr_mode = data[0];
    // MADCTL
    read_data(ILI9341_RDMADCTL, data, 2);
    __ili9341_disp_info.madctl = data[0];
    // Pixel Format
    read_data(ILI9341_RDPIXFMT, data, 2);
    __ili9341_disp_info.pixelfmt = data[0];
    // Image Format
    read_data(ILI9341_RDIMGFMT, data, 2);
    __ili9341_disp_info.imagefmt = data[0];
    // Signal Mode
    read_data(ILI9341_RDSIGMODE, data, 2);
    __ili9341_disp_info.signal_mode = data[0];
    // Selftest result
    read_data(ILI9341_RDSELFDIAG, data, 2);
    __ili9341_disp_info.selftest = data[0];

    return (&__ili9341_disp_info);
}
