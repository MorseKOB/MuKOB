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

#define ILI9341_WIDTH 320    // -  ILI9341 display width
#define ILI9341_HEIGHT 240   // -  ILI9341 display height

// Command descriptions start on page 83 of the datasheet

#define ILI9341_NOP         0x00    // -  No-op
#define ILI9341_SWRESET     0x01    // -  Software reset
#define ILI9341_RDDID       0x04    // -  Read display identification information (3)
#define ILI9341_RDDST       0x09    // -  Read Display Status (2)

#define ILI9341_SLPIN       0x10    // -  Enter Sleep Mode
#define ILI9341_SLPOUT      0x11    // -  Sleep Out
#define ILI9341_PTLON       0x12    // -  Partial Mode ON
#define ILI9341_NORON       0x13    // -  Normal Display Mode ON

#define ILI9341_RDMODE      0x0A    // -  Read Display Power Mode (1)
#define ILI9341_RDMADCTL    0x0B    // -  Read Display MADCTL (1)
#define ILI9341_RDPIXFMT    0x0C    // -  Read Display Pixel Format (1)
#define ILI9341_RDIMGFMT    0x0D    // -  Read Display Image Format (1)
#define ILI9341_RDSIGMODE   0x0E    // -  Read Display Signal Mode (1)
#define ILI9341_RDSELFDIAG  0x0F    // -  Read Display Self-Diagnostic Result (1)
#define ILI9341_RDID1       0xDA    // -  Read Display Manufacturer ID (1)
#define ILI9341_RDID2       0xDB    // -  Read Display Version ID (1)
#define ILI9341_RDID3       0xDC    // -  Read Display Module/Driver ID (1)
#define ILI9341_RDID4       0xD3    // -  Read Display IC (Ver, Model-A, Model-B) (3)

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

#define ILI9341_GMCTLP1     0xE0    // -  Positive Gamma Correction
#define ILI9341_GMCTLN1     0xE1    // -  Negative Gamma Correction
#define ILI9341_PWCTL6      0xFC    // -  Power Control


// 24-bit Color Mode to Basic 16 Colors (match original PC VGA system)
//                                      NUM :   R    G    B
//                                      --- : ---  ---  ---
#define ILI9341_BLACK       0x000000  //  0 :   0,   0,   0
#define ILI9341_BLUE        0x0000AA  //  1 :   0,   0, 170
#define ILI9341_GREEN       0x4E9100  //  2 :  78, 145,   0
#define ILI9341_CYAN        0x00FAFF  //  3 :   0, 250, 255
#define ILI9341_RED         0xAA0000  //  4 : 170,   0,   0
#define ILI9341_MAGENTA     0xFF40FF  //  5 : 255,  64, 255
#define ILI9341_BROWN       0x641100  //  6 : 100,  17,   0
#define ILI9341_WHITE       0xD0D0D0  //  7 : 208, 208, 208
#define ILI9341_GREY        0x79795A  //  8 : 121, 121,  90
#define ILI9341_LT_BLUE     0x0064FF  //  9 :   0, 100, 255
#define ILI9341_LT_GREEN    0x00F900  // 10 :   0, 249,   0
#define ILI9341_LT_CYAN     0x73FDFF  // 11 : 115, 253, 255
#define ILI9341_ORANGE      0xFF4B02  // 12 : 255,  75,   2
#define ILI9341_LT_MAGENTA  0xFF8AD8  // 13 : 255, 138, 218
#define ILI9341_YELLOW      0xFF9300  // 12 : 255, 147,   0
#define ILI9341_BR_WHITE    0xFFFFFF  // 15 : 255, 255, 255

/*! \brief RED/GREEN/BLUE */
typedef struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

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
    // ID1 (MFG CMD 0xDA)
    uint8_t lcd_id1_mfg;
    // ID2 (Version CMD 0xDB)
    uint8_t lcd_id2_ver;
    // ID3 (Driver CMD 0xDB)
    uint8_t lcd_id3_drv;
    // ID4 (IC Ver,Mdl,Mdl CMD 0xD3)
    uint8_t lcd_id4_icv;
    uint8_t lcd_id4_icm1;
    uint8_t lcd_id4_icm2;
} ili9341_disp_info_t;

/**
 * \brief Get a pointer to a buffer large enough to hold 
 * one scan line for the ILI9341 display.
 * 
 * This can be used to put RGB data into to be written to the 
 * screen. The `ili9341_line_paint` can be called to put the 
 * line on the screen.
 * 
 * The buffer holds `ILI9341_WIDTH` rgb_t values.
*/
rgb_t* ili9341_get_line_buf();

/**
 * \brief Paint a buffer of rgb_t values to one horizontal line 
 * of the screen. The buffer passed in must be at least `ILI9341_WIDTH` 
 * rgb_t values in size.
 * \ingroup display
 * 
 * \param line 0-based line number to paint (must be less than `ILI9341_WIDTH`).
 * \param buf pointer to a rgb_t buffer of data (must be at least 'ILI9341_WIDTH`)
*/
void ili9341_line_paint(uint16_t line, rgb_t *buf);

/**
 * \brief Initialize the display.
 * \ingroup display
*/
void ili9341_spi_init(void);

/**
 * \brief Get information about the display hardare & configuration.
 * \ingroup display
*/
ili9341_disp_info_t* ili9341_spi_info(void);

/**
 * \brief Clear the entire screen.
 * \ingroup display
*/
void ili9341_screen_clr(void);

/**
 * \brief Paint the screen with the RGB contents of a buffer.
 * \ingroup display
 * 
 * Uses the buffer of RGB data to paint the screen into the screen window.
 * Set the screen window using `ili9341_set_window`.
 * 
 * \param data RGB pixel data buffer (1 rgb value for each pixel to paint)
 * \param pixels Number of pixels (size of the data buffer in rgb_t's)
*/
void ili9341_screen_paint(const rgb_t *rgb_pixel_data, uint16_t pixels);

/**
 * \brief Set the screen update window and position the start at x,y.
 * \ingroup display
 * 
 * Sets the update window area on the screen. This is the area that RGB data 
 * will be updated into using `ili9341_screen_paint`.
*/
void ili9341_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * \brief Set the screen update window to the full screen, and position the 
 * start at 0,0.
 * \ingroup display
 * 
 * Sets the update window to the full screen and positions the start at 0,0.
*/
void ili9341_set_window_fullscreen(void);

#ifdef __cplusplus
 }
#endif
#endif  // ILI9341_SPI_H
