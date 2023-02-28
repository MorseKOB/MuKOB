/**
 * @brief Test, demo, and debugging routines.
 * @ingroup test
 *
 * Many of these aren't actually tests. Rather, they tend to be routines
 * that display patterns, send things to the terminal, get rotory control
 * values, etc., that can be helpful for seeing, demo'ing, and debugging
 * functionality.
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _TEST_H_
#define _TEST_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Test printing an error to the terminal.
 */
void test_error_printf();

/**
 * @brief Show a full screen scrolling barber pole (font pattern)
 */
void test_disp_show_full_scroll_barberpoll();

/**
 * @brief Show a half width scrolling barber pole (font pattern)
 *
 */
void test_disp_show_half_width_scroll_barberpoll();

/**
 * @brief Fill a screen with a pattern and then use the ILI9341 scroll
 */
void test_ili9341_show_scroll();

/**
 * @brief Show the MuKOB header and footer (mock)
 *
 * Also sets the scroll window to lines 2-18.
 *
 */
void test_disp_show_mukob_head_foot();

/**
 * @brief Send a color chart to the terminal.
 *
 */
void test_term_color_chart();

/**
 * @brief Set the screen & page size to 132 x 48 and print a diagonal.
 * 
 */
void test_term_screen_page_size();

#ifdef __cplusplus
}
#endif
#endif // _TEST_H_
