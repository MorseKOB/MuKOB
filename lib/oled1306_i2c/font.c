/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "font.h"

//
// Font data for Terminal (fixed-pitch) 9 x 10
//
// 32 special/graphic characters, then standard ASCII
//
// The characters are layed out on their 'side' so the data can be
// loaded onto the 1306-based display more easily. Each character
// takes 2 bytes:
// +---+---+++---+---+---+---++---+---+---+---+
// | 9 | 8 ||| 7 | 6 | 5 | 4 || 3 | 2 | 1 | 0 |
// +---+---+++---+---+---+---++---+---+---+---|
//

const uint16_t Font_Table[] =
{
	// 0x00 < NULL > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x01 < Closer - Closed > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0180, //   # #
	0x01E0, //   # # # # 
	0x0140, //   #   #  
	0x0140, //   #   #  
	0x0140, //   #   #  
	0x0150, //   #   #   #  
	0x01F0, //   # # # # #
	0x0150, //   #   #   #
	0x0000, //

	// 0x02 < Closer - Open > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0100, //   # 
	0x01C0, //   # # # 
	0x0140, //   #   #     
	0x0128, //   #     #   #
	0x0114, //   #       #   #
	0x010A, //   #         #   #
	0x0194, //   # #     #   # 
	0x0100, //   #    
	0x0000, //

	// 0x03 < WiFi - Not Connected > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x01C4, //   # # #       #
	0x00EA, //     # # #   #   #
	0x0075, //       # # #   #   #
	0x01FD, //   # # # # # # #   #
	0x019D, //   # #     # # #   #
	0x002F, //         #   # # # #
	0x000F, //             # # # #
	0x0003, //                 # #
	0x0000, //                 

	// 0x04 < WiFi - Connected > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0004, //               #
	0x000A, //             #   #
	0x0025, //         #     #   #
	0x0195, //   # #     #   #   #
	0x0195, //   # #     #   #   #
	0x0025, //         #     #   #
	0x000A, //             #   #
	0x0004, //               #
	0x0000, //

	// 0x05 <  SAVE > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x00C0, //     # #      
	0x01E0, //   # # # #     
	0x0164, //   #   # #     #  
	0x012F, //   #     #   # # # #
	0x0124, //   #     #     # 
	0x01A0, //   # #   #
	0x01E0, //   # # # #
	0x00C0, //     # # 
	0x0000, //

	// 0x06 < Check Box > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x01FF, //   # # # # # # # # #
	0x0101, //   #               #
	0x0101, //   #               #
	0x0101, //   #               #
	0x0101, //   #               #
	0x0101, //   #               #
	0x0101, //   #               #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x07 < Check Box - Selected > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x01FF, //   # # # # # # # # #
	0x0101, //   #               #
	0x017D, //   #   # # # # #   #
	0x017D, //   #   # # # # #   #
	0x017D, //   #   # # # # #   #
	0x017D, //   #   # # # # #   #
	0x0101, //   #               #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x08 < BS > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0010, //           #
	0x0038, //         # # #
	0x0054, //       #   #   #
	0x0092, //     #     #     #
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x0000, //

	// 0x09 < OK (ACK) > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0040, //       #
	0x0080, //     #
	0x0180, //   # #
	0x0060, //       # #
	0x0018, //           # #
	0x0006, //               # #
	0x0001, //                   #
	0x0000, //             
	0x0000, //

	// 0x0A < Radio Button > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0078, //       # # # #
	0x0084, //     #         #
	0x0102, //   #             #
	0x0102, //   #             #
	0x0102, //   #             #
	0x0102, //   #             #
	0x0084, //     #         # 
	0x0078, //       # # # #
	0x0000, //

	// 0x0B < Radio Button - Selected > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0078, //       # # # #
	0x0084, //     #         #
	0x0132, //   #     # #     #
	0x017A, //   #   # # # #   #
	0x017A, //   #   # # # #   #
	0x0132, //   #     # #     #
	0x0084, //     #         # 
	0x0078, //       # # # #
	0x0000, //

	// 0x0C < Home > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0008, //             #
	0x01FC, //   # # # # # # #
	0x01EE, //   # # # #   # # #
	0x007F, //       # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x01EE, //   # # # #   # # #
	0x01FC, //   # # # # # # #
	0x0008, //             #
	0x0000, //

	// 0x0D < ARROW - LEFT > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0010, //           #
	0x0038, //         # # #
	0x007C, //       # # # # #
	0x00FE, //     # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x0000, //
	0x0000, //

	// 0x0E <  > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x01FF, //   # # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x0F < ARROW - RIGHT > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x01FF, //   # # # # # # # # #
	0x00FE, //     # # # # # # #
	0x007C, //       # # # # #
	0x0038, //         # # #
	0x0010, //           #
	0x0000, //
	0x0000, //

	// 0x10 < DELETE > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0006, //               # #
	0x00F9, //     # # # # #     #
	0x01F9, //   # # # # # #     #
	0x0109, //   #         #     #
	0x01F9, //   # # # # # #     #
	0x0109, //   #         #     #
	0x00F9, //     # # # # #     #
	0x0006, //               # #
	0x0000, //

	// 0x11 < UP/DOWN > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0044, //       #       #
	0x00C6, //     # #       # #
	0x01FF, //   # # # # # # # # #
	0x00C6, //     # #       # #
	0x0044, //       #       #
	0x0000, //
	0x0000, //

	// 0x12 < LEFT/RIGHT > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0010, //           #
	0x0038, //         # # #
	0x007C, //       # # # # #
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x007C, //       # # # # #
	0x0038, //         # # #
	0x0010, //           #
	0x0000, //

	// 0x13 < ACTIVITY - 1 > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0180, //   # #
	0x01C0, //   # # #
	0x01A0, //   # #   #
	0x0190, //   # #     #
	0x0188, //   # #       #
	0x0184, //   # #         #
	0x0182, //   # #           #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x14 < ACTIVITY - 2 > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0180, //   # #
	0x01C0, //   # # #
	0x01E0, //   # # # #
	0x01D0, //   # # #   # 
	0x01C8, //   # # #     #
	0x01C4, //   # # #       #
	0x01C2, //   # # #         #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x15 < ACTIVITY - 3 > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0180, //   # #
	0x01C0, //   # # #
	0x01E0, //   # # # #
	0x01F0, //   # # # # #
	0x01E8, //   # # # #   #
	0x01E4, //   # # # #     #
	0x01E2, //   # # # #       #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x16 < ACTIVITY - 4 > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0180, //   # #
	0x01C0, //   # # #
	0x01E0, //   # # # #
	0x01F0, //   # # # # #
	0x01F8, //   # # # # # #
	0x01F4, //   # # # # #   #
	0x01F2, //   # # # # #     #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x17 < ACTIVITY - 5 (Sleeping) > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0180, //   # #
	0x01C0, //   # # #
	0x01E0, //   # # # #
	0x01F0, //   # # # # #
	0x01F8, //   # # # # # #
	0x01FC, //   # # # # # # #
	0x01FE, //   # # # # # # # #
	0x01FF, //   # # # # # # # # #
	0x0000, //

	// 0x18 < CANCEL (CAN) > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0082, //     #           #
	0x0044, //       #       #
	0x0028, //         #   #
	0x0010, //           #
	0x0028, //         #   #
	0x0044, //       #       #
	0x0082, //     #           #
	0x0000, //

	// 0x19 < ACTIVITY - 0 (Sleeping) > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0110, //   #       #
	0x0190, //   # #     #
	0x0150, //   #   #   #
	0x0131, //   #     # #       #
	0x0119, //   #       # #     #
	0x0015, //           #   #   #
	0x0013, //           #     # #
	0x0011, //           #       #
	0x0000, //

	// 0x1A ' ' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x1B ' ' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x1C ' ' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x1D < Paragraph ('='-0x20) > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0101, //   #               #
	0x01FF, //   # # # # # # # # #
	0x0101, //   #               #
	0x01FF, //   # # # # # # # # #
	0x0121, //   #     #         #
	0x0021, //         #         #
	0x001E, //           # # # #
	0x000C, //             # #
	0x0000, //

	// 0x1E < Menu (Hamburger) > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //           
	0x0092, //     #     #     #
	0x0092, //     #     #     #
	0x0092, //     #     #     #
	0x0092, //     #     #     #
	0x0092, //     #     #     #
	0x0092, //     #     #     #
	0x0092, //     #     #     #
	0x0000, //

	// 0x1F <  > (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0054, //       #   #   #
	0x00AA, //     #   #   #   #
	0x007C, //       # # # # #
	0x0145, //   #   #       #   #
	0x0145, //   #   #       #   #
	0x007C, //       # # # # #
	0x00AA, //     #   #   #   #
	0x0054, //       #   #   #
	0x0000, //

	// 0x20 ' ' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x21 '!' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x01BF, //   # #   # # # # # #   
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x22 '"' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0007, //               # # #
	0x0003, //                 # #
	0x0000, //
	0x0007, //               # # #
	0x0003, //                 # #
	0x0000, //
	0x0000, //

	// 0x23 '#' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0028, //         #   #
	0x01A8, //   # #   #   #
	0x007C, //       # # # # #
	0x002B, //         #   #   # #
	0x01A8, //   # #   #   #
	0x007C, //       # # # # #
	0x002B, //         #   #   # #
	0x0028, //         #   #  
	0x0000, //

	// 0x24 '$' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0044, //       #       #
	0x008A, //     #       #   #
	0x0092, //     #     #     #
	0x01D7, //   # # #   #   # # #
	0x0092, //     #     #     #
	0x0094, //     #     #   #
	0x0060, //       # #
	0x0000, //

	// 0x25 '%' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //               
	0x0082, //     #           #
	0x0045, //       #       #   #
	0x0022, //         #       #
	0x0010, //           #
	0x0088, //     #       #
	0x0144, //   #   #       #
	0x0082, //     #           #
	0x0000, //

	// 0x26 '&' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //       
	0x00EC, //     # # #   # #
	0x0112, //   #       #     #
	0x0122, //   #     #       #
	0x0144, //   #   #       #
	0x0080, //     # 
	0x0140, //   #   #
	0x0000, //
	0x0000, //

	// 0x27 ''' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0007, //               # # #
	0x0003, //                 # #
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x28 '(' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, // 
	0x0000, //
	0x0038, //         # # #
	0x00C6, //     # #       # #
	0x0101, //   #               #
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x29 ')' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0101, //   #               #
	0x00C6, //     # #       # #
	0x0038, //         # # #
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x2A '*' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0082, //     #           #
	0x0044, //       #       #
	0x0038, //         # # #
	0x01FF, //   # # # # # # # # #
	0x0038, //         # # #
	0x0044, //       #       #
	0x0082, //     #           #
	0x0000, //

	// 0x2B '+' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x00FE, //     # # # # # # #
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x0000, //

	// 0x2C ',' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0200, // # 
	0x0180, //   # #
	0x0080, //     #
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x2D '-' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x0010, //           #
	0x0000, //
	0x0000, //

	// 0x2E '.' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0180, //   # #
	0x0180, //   # #
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x2F '/' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //   
	0x0180, //   # #
	0x0060, //       # #
	0x0018, //           # #
	0x0006, //               # #
	0x0001, //                   #
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x30 '0' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //              
	0x007C, //       # # # # #
	0x0082, //     #           #
	0x0101, //   #               #
	0x0101, //   #               #
	0x0082, //     #           #
	0x007C, //       # # # # #
	0x0000, //
	0x0000, //

	// 0x31 '1' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0101, //   #               #
	0x01FF, //   # # # # # # # # #
	0x0100, //   #
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x32 '2' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x01C2, //   # # #         #
	0x0121, //   #     #         #
	0x0111, //   #       #       #
	0x0109, //   #         #     #
	0x0106, //   #           # #
	0x0100, //   #
	0x0000, //
	0x0000, //

	// 0x33 '3' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0082, //     #           #
	0x0101, //   #               #
	0x0109, //   #         #     #
	0x0109, //   #         #     #
	0x0116, //   #       #   # #
	0x00E0, //     # # #      
	0x0000, //
	0x0000, //

	// 0x34 '4' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //  
	0x0060, //       # #
	0x0050, //       #   #
	0x0048, //       #     #
	0x0044, //       #       #
	0x0042, //       #         #
	0x01FF, //   # # # # # # # # #
	0x0040, //       #
	0x0000, //

	// 0x35 '5' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x008F, //     #       # # # #
	0x0109, //   #         #     #
	0x0109, //   #         #     #
	0x0109, //   #         #     #
	0x0109, //   #         #     #
	0x00F0, //     # # # # 
	0x0000, //
	0x0000, //

	// 0x36 '6' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x00FC, //     # # # # # #  
	0x0122, //   #     #       #
	0x0111, //   #       #       #
	0x0111, //   #       #       #
	0x0111, //   #       #       #
	0x00E0, //     # # # 
	0x0000, //                 
	0x0000, //

	// 0x37 '7' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0001, //                   #
	0x0001, //                   #
	0x0181, //   # #             #
	0x0061, //       # #         #
	0x0019, //           # #     #
	0x0007, //               # # #
	0x0000, //
	0x0000, //

	// 0x38 '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x39 '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x3A '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x3B '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x3C '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x3D '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x3E '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

	// 0x3F '' (9 wide x 10 high cell)
	//      // 9 8|7 6 5 4|3 2 1 0
	//      // - -|- - - -|- - - -
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //
	0x0000, //

};
