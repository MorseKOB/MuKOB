/**
 * @brief Terminal functionality.
 * @ingroup display
 *
 * This provides a very (very) simple NCURSES-like functionality.
 * It is hard-coded to expect an ANSI capable terminal. It is tested primarily
 * with Putty.
 *
 * @see https://www.putty.org/ (Windows)
 * @see https://www.puttygen.com/download-putty#Download_PuTTY_for_Mac_and_Installation_Guide (Mac)
 * @see https://www.puttygen.com/download-putty#Download_PuTTY_on_Linux_and_Installation_Guide (Linux)
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "term.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"

/*
 * The following are from the Dec VT-510 programmer's manual.
 *
 * Setting Control Sequence             Final Chars     Mnemonic
 * ==================================== =============== ==========
 * Select Active Status Display         $ g             DECSASD
 * Select Attribute Change Extent       * x             DECSACE
 * Set Character Attribute              " q             DECSCA
 * Set Conformance Level                " p             DECSCL
 * Set Columns Per Page                 $ |             DECSCPP
 * Set Lines Per Page                   t               DECSLPP
 * Set Number of Lines per Screen       * |             DECSNLS
 * Set Status Line Type                 $ ~             DECSSDT
 * Set Left and Right Margins           s               DECSLRM
 * Set Top and Bottom Margins           r               DECSTBM
 * Set Graphic Rendition                m               SGR
 * Select Set-Up Language               p               DECSSL
 * Select Printer Type                  $ s             DECSPRTT
 * Select Refresh Rate                  " t             DECSRFR
 * Select Digital Printed Data Type     ) p             DECSDPT
 * Select ProPrinter Character Set      * p             DECSPPCS
 * Select Communication Speed           * r             DECSCS
 * Select Communication Port            * u             DECSCP
 * Set Scroll Speed                     SP p            DECSSCLS
 * Set Cursor Style                     SP q            DECSCUSR
 * Set Key Click Volume                 SP r            DECSKCV
 * Set Warning Bell Volume              SP t            DECSWBV
 * Set Margin Bell Volume               SP u            DECSMBV
 * Set Lock Key Style                   SP v            DECSLCK
 * Select Flow Control Type             * s             DECSFC
 * Select Disconnect Delay Time         $ q             DECSDDT
 * Set Transmit Rate Limit              " u             DECSTRL
 * Set Port Parameter                   + w             DECSPP
 *
 */
#undef putc     // Use the function, not the macro
#undef putchar  // Use the function, not the macro

#define _INPUT_BUF_SIZE_ 32 // Make this a power of 2

static char _input_buf[_INPUT_BUF_SIZE_];
static bool _input_buf_overflow = false;
static uint16_t _input_buf_in = 0;
static uint16_t _input_buf_out = 0;

/**
 * @brief Callback function that is registered with the STDIO handler to be notified when characters become available.
 *
 * @param param Value that is passed to us that we registered with.
 */
static void _stdio_chars_available(void *param) {
    // Read the character
    int i;
    do {
        i = getchar_timeout_us(0); // 0 means return immediately if not available
        if (i == PICO_ERROR_TIMEOUT) {
            break;
        }
        // See if we have room for it in the input buffer
        if (((_input_buf_in + 1) % _INPUT_BUF_SIZE_) == _input_buf_out) {
            // No room. Just throw it away.
            _input_buf_overflow = true;
            break;
        }
        _input_buf[_input_buf_in] = (char)i;
        _input_buf_in = (_input_buf_in + 1) % _INPUT_BUF_SIZE_;
    } while (true);
}

inline void term_clear() {
    printf("%s2J", CSI); // Some guides indicate 'ESC c', but that is 'Reset' not just 'Clear'
}

inline void term_color_default() {
    printf("%s39;49m", CSI);
}

inline void term_color_bg(term_color_t colorn) {
    printf("%s48;5;%dm", CSI, colorn);
}

inline void term_color_fg(term_color_t colorn) {
    printf("%s38;5;%dm", CSI, colorn);
}

inline void term_cursor_moveto(uint16_t line, uint16_t column) {
    printf("%s%hd;%hdH", CSI, line, column);
}

inline void term_cursor_restore() {
    printf("%c8", ESC);
}

inline void term_cursor_save() {
    printf("%c7", ESC);
}

void term_cursor_on(bool on) {
    char onoff = (on ? 'h' : 'l');
    printf("%s?25%c", CSI, onoff); // VT220
}

int term_getc(void) {
    if (!term_input_available()) {
        return (-1);
    }
    int c = _input_buf[_input_buf_out];
    _input_buf_out = (_input_buf_out + 1) % _INPUT_BUF_SIZE_;

    return (c);
}

void term_init() {
    // Terminal type and screen size...
    term_reset();
    sleep_ms(100); // Allow the terminal to reset
    term_set_type_vt(VT_510_TYPE_SPEC);
    term_set_size(48, 132);
    term_clear();
    term_color_default();
    term_cursor_on(true);

    // Input handler...
    stdio_set_chars_available_callback(_stdio_chars_available, NULL);   // We can pass a parameter if we want
}

inline bool term_input_available() {
    return (_input_buf_in != _input_buf_out);
}

void term_input_buf_clear(void) {
    _input_buf_in = _input_buf_out = 0;
    _input_buf_overflow = false;
}

bool term_input_overflow() {
    bool retval = _input_buf_overflow;
    _input_buf_overflow = false;

    return (retval);
}

inline void term_reset() {
    printf("%cc", ESC);
}

void term_set_size(uint16_t lines, uint16_t columns) {
    // The Dec manual says :
    // Note 1. The page size can be 24, 25, 36, 42, 48, 52, and 72 lines with 80 or 132 columns.
    // (PuTTY accepts any value)
    char colind = 'l'; // 80 columns

    // Check and adjust columns...
    if (columns <= 80) {
        columns = 80;
    }
    else if (columns > 80) {
        columns = 132;
        colind = 'h';
    }

    // Check and adjust lines...
    if (lines <= 24) {
        lines = 24;
    }
    else if (lines >= 72) {
        lines = 72;
    }
    else if (lines >= 52) {
        lines = 52;
    }
    else if (lines >= 48) {
        lines = 48;
    }
    else if (lines >= 42) {
        lines = 42;
    }
    else if (lines >= 36) {
        lines = 36;
    }
    else {
        lines = 25;
    }

    // Send the terminal the commands to set the screen and the page size...
    printf("%s?3%c%s%hd$|", CSI, colind, CSI, columns); // Columns: Screen size | Page size
    printf("%s%hd*|%s%hdt", CSI, lines, CSI, lines); // Lines: Screen size | Page size
}

inline void term_set_type_vt(vt_term_type_spec_t type) {
    printf("%s6%d;1;\"p", CSI, type); // The ;1 specifies using 7-bit control mode
    // printf("%s41;1;\"p", CSI); // Set to PuTTY mode ('41' is PuTTY only value)
    // printf("%s42;1;\"p", CSI); // Set to SCO-ANSI mode ('42' is PuTTY only value)
}