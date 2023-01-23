/**
 * Copyright 2023 AESilky
 * 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DISPLAY_H
#define DISPLAY_H
#ifdef __cplusplus
 extern "C" {
#endif

#define DISP_CHAR_ROWS 6
#define DISP_CHAR_COLS 14

/** \brief Initialize the display
 * 
 * This must be called before using the display.
 *
 */
void display_init(void);

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

/*! \brief Test the fonts by displaying all of the characters
 *  \ingroup display
 *
 * Display all of the font characters a page at a time. Pause between pages and overlap 
 * the range of characters some from page to page.
 * 
 */
void disp_font_test(void);

#ifdef __cplusplus
}
#endif
#endif // DISPLAY_H

