/**
 * ILI9341 320x240 Color LCD functionaly interface through SPI
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 * 
 */
#ifndef ILI9341_SPI_H
#define ILI9341_SPI_H
#ifdef __cplusplus
 extern "C" {
#endif

#define ILI9341_WIDTH 240    // -  ILI9341 display width
#define ILI9341_HEIGHT 320   // -  ILI9341 display height

// Command descriptions start on page 83 of the datasheet

#define ILI9341_NOP         0x00    // -  No-op
#define ILI9341_SWRESET     0x01    // -  Software reset
#define ILI9341_RDDID       0x04    // -  Read display identification information
#define ILI9341_RDDST       0x09    // -  Read Display Status

#define ILI9341_SLPIN       0x10    // -  Enter Sleep Mode
#define ILI9341_SLPOUT      0x11    // -  Sleep Out
#define ILI9341_PTLON       0x12    // -  Partial Mode ON
#define ILI9341_NORON       0x13    // -  Normal Display Mode ON

#define ILI9341_RDMODE      0x0A    // -  Read Display Power Mode
#define ILI9341_RDMADCTL    0x0B    // -  Read Display MADCTL
#define ILI9341_RDPIXFMT    0x0C    // -  Read Display Pixel Format
#define ILI9341_RDIMGFMT    0x0D    // -  Read Display Image Format
#define ILI9341_RDSIGMODE   0x0E    // -  Read Display Signal Mode
#define ILI9341_RDSELFDIAG  0x0F    // -  Read Display Self-Diagnostic Result

#define ILI9341_INVOFF      0x20    // -  Display Inversion OFF
#define ILI9341_INVON       0x21    // -  Display Inversion ON
#define ILI9341_GAMMASET    0x26    // -  Gamma Set
#define ILI9341_DISPOFF     0x28    // -  Display OFF
#define ILI9341_DISPON      0x29    // -  Display ON

#define ILI9341_CASET       0x2A    // -  Column Address Set
#define ILI9341_PASET       0x2B    // -  Page Address Set
#define ILI9341_RAMWR       0x2C    // -  Memory Write
#define ILI9341_RAMRD       0x2E    // -  Memory Read

#define ILI9341_PTLAR       0x30    // -  Partial Area
#define ILI9341_VSCRDEF     0x33    // -  Vertical Scrolling Definition
#define ILI9341_MADCTL      0x36    // -  Memory Access Control
#define ILI9341_VSCRSADD    0x37    // -  Vertical Scrolling Start Address
#define ILI9341_PIXFMT      0x3A    // -  COLMOD: Pixel Format Set

#define ILI9341_FRMCTL1     0xB1    // -  Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTL2     0xB2    // -  Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTL3     0xB3    // -  Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTL      0xB4    // -  Display Inversion Control
#define ILI9341_DFUNCTL     0xB6    // -  Display Function Control

#define ILI9341_PWCTL1      0xC0    // -  Power Control 1
#define ILI9341_PWCTL2      0xC1    // -  Power Control 2
#define ILI9341_PWCTL3      0xC2    // -  Power Control 3
#define ILI9341_PWCTL4      0xC3    // -  Power Control 4
#define ILI9341_PWCTL5      0xC4    // -  Power Control 5
#define ILI9341_VMCTL1      0xC5    // -  VCOM Control 1
#define ILI9341_VMCTL2      0xC7    // -  VCOM Control 2

#define ILI9341_RDID1       0xDA    // -  Read ID 1
#define ILI9341_RDID2       0xDB    // -  Read ID 2
#define ILI9341_RDID3       0xDC    // -  Read ID 3
#define ILI9341_RDID4       0xDD    // -  Read ID 4

#define ILI9341_GMCTLP1     0xE0    // -  Positive Gamma Correction
#define ILI9341_GMCTLN1     0xE1    // -  Negative Gamma Correction
#define ILI9341_PWCTL6      0xFC    // -  


// 4-bit Color Mode Basic 16 Colors (match original PC VGA system)
#define ILI9341_BLACK     0x0000  //  0 :   0,   0,   0
#define ILI9341_BLUE      0x001F  //  1 :   0,   0, 255
#define ILI9341_GREEN     0x07E0  //  2 :   0, 255,   0
#define ILI9341_CYAN      0x07FF  //  3 :   0, 255, 255
#define ILI9341_RED       0xF800  //  4 : 255,   0,   0
#define ILI9341_MAGENTA   0xF81F  //  5 : 255,   0, 255
#define ILI9341_BROWN     0xF81F  //  6 : 255,   0, 255
#define ILI9341_WHITE     0xFFFF  //  7 : 127, 127, 127
#define ILI9341_GREY      0xC618  //  8 : 198, 195, 198
#define ILI9341_LT_BLUE   0x000F  //  9 :   0,   0, 123
#define ILI9341_LT_GREEN  0xAFE5  // 10 : 173, 255,  41
#define ILI9341_LT_CYAN   0x03EF  // 11 :   0, 125, 123
#define ILI9341_ORANGE    0xFD20  // 12 : 255, 165,   0
#define ILI9341_VIOLET    0x780F  // 13 : 123,   0, 123
#define ILI9341_YELLOW    0xFFE0  // 14 : 255, 255,   0
#define ILI9341_BR_WHITE  0xFFFF  // 15 : 255, 255, 255


/**
 * Display information.
 */
typedef struct ili9341_disp_info {
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
} ili9341_disp_info_t;

/**
 * \brief Initialize the display
*/
void ili9341_spi_init(void);

/**
 * \brief Get information about the display
*/
ili9341_disp_info_t* ili9341_spi_info(void);

#ifdef __cplusplus
 }
#endif
#endif  // ILI9341_SPI_H
