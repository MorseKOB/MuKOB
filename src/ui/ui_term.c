/**
 * MuKOB User Interface - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_term.h"
#include "config.h"
#include "cmt.h"
#include "term.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"
#include <ctype.h>
#include <string.h>

#define _UI_TERM_GETLINE_MAX_LEN_  256 // Maximum line length (including /0) for the `term_getline` function

static term_color_t _color_term_text_current_bg;
static term_color_t _color_term_text_current_fg;
static ui_term_control_char_handler _control_char_handler[32]; // Room for a handler for each control character
static char _getline_buf[_UI_TERM_GETLINE_MAX_LEN_];
static int16_t _getline_index;

static ui_term_input_available_handler _input_available_handler;
static ui_term_getline_callback_fn _getline_callback; // Function pointer to be called when an input line is ready

/**
 * @brief Message handler for `MSG_INPUT_CHAR_READY`
 * @ingroup ui
 *
 * @param msg Nothing important in the message.
 */
void _ui_term_handle_input_char_ready(cmt_msg_t* msg) {
    if (NULL != _input_available_handler) {
        _input_available_handler();
    }
}

/**
 * @brief A `term_notify_on_input_fn` handler for input ready.
 * @ingroup ui
 */
void _input_ready_hook(void) {
    // Since this is called by an interrupt handler,
    // post a UI message so that the input is handled
    // by the UI message loop.
    cmt_msg_t msg;
    msg.id = MSG_INPUT_CHAR_READY;
    postUIMsgBlocking(&msg);
    // The hook is cleared on notify, so hook ourself back in.
    term_register_notify_on_input(_input_ready_hook);
}

static void _header_fill_fixed() {
    term_color_fg(UI_TERM_HEADER_COLOR_FG);
    term_color_bg(UI_TERM_HEADER_COLOR_BG);
    term_cursor_save();
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, 1);
    term_erase_line();
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_WIRE_LABEL_COL);
    printf("%s", UI_TERM_WIRE_LABEL);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_SPEED_LABEL_COL);
    printf("%s", UI_TERM_SPEED_LABEL);
    term_color_default();
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

static void _status_fill_fixed() {
    term_cursor_save();
    term_color_fg(UI_TERM_STATUS_COLOR_FG);
    term_color_bg(UI_TERM_STATUS_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_STATUS_LINE, 1);
    term_erase_line();
    printf("%s", UI_TERM_NAME_VERSION);
    term_cursor_moveto(UI_TERM_STATUS_LINE, UI_TERM_STATUS_LOGO_COL);
    printf("%s", AES_LOGO);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

static void _ui_term_getline_continue() {
    int ci;
    ui_term_getline_callback_fn fn = _getline_callback;

    // Process characters that are available.
    while ((ci = term_getc()) >= 0) {
        char c = (char)ci;
        if ('\n' == c || '\r' == c) {
            // EOL - Terminate the input line and give to callback.
            _getline_buf[_getline_index] = '\0';
            _getline_index = 0;
            _getline_callback = NULL;
            ui_term_register_input_available_handler(NULL);
            fn(_getline_buf);
            return;
        }
        if (BS == c || DEL == c) {
            // Backspace/Delete - move back if we aren't at the BOL
            if (_getline_index > 0) {
                _getline_index--;
                term_cursor_left_1();
                term_erase_char(1);
            }
            _getline_buf[_getline_index] = '\0';
        }
        else if (ESC == c) {
            // Escape - erase the line
            while (_getline_index > 0) {
                _getline_buf[_getline_index] = '\0';
                term_cursor_left_1();
                term_erase_char(1);
                _getline_index--;
            }
            _getline_index = 0;
            _getline_buf[_getline_index] = '\0';
            // If there is a handler registered for ESC let it handle it too
            ui_term_handle_control_character(c);
        }
        else if (c >= ' ' && c < DEL) {
            if (_getline_index < (_UI_TERM_GETLINE_MAX_LEN_ - 1)) {
                _getline_buf[_getline_index++] = c;
                putchar(c);
            }
            else {
                // Alert them that they are at the end
                putchar(BEL);
            }
        }
        else {
            // See if there is a handler registered for this, else BEEP
            if (!ui_term_handle_control_character(c)) {
                // Control or 8-bit character we don't deal with
                putchar(BEL);
            }
        }
        // `while` will see if there are more chars available
    }
    // No more input chars are available, but we haven't gotten EOL yet,
    // hook for more to wake back up...
    term_register_notify_on_input(_input_ready_hook);
}

static void _term_init() {
    _input_available_handler = NULL;
    memset(_control_char_handler, 0, sizeof(_control_char_handler));
    term_reset();
    term_color_default();
    term_set_type(VT_510_TYPE_SPEC, VT_510_ID_SPEC);
    term_set_title(UI_TERM_NAME_VERSION);
    term_set_size(UI_TERM_LINES, UI_TERM_COLUMNS);
    term_clear();
    term_set_margin_top_bottom(UI_TERM_SCROLL_START_LINE, UI_TERM_SCROLL_END_LINE);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_clear();
    term_cursor_on(false);
    ui_term_use_code_color();
    // ZZZ Temp to count lines and see scroll...
    for (int i = 1; i <= UI_TERM_LINES; i++) {
        printf("Line %d\n", i);
    }
}

void ui_term_build(void) {
    _term_init();
    _header_fill_fixed();
    _status_fill_fixed();
    ui_term_display_speed();
    ui_term_display_wire();
    ui_term_update_sender(NULL); // Build a blank sender line
    ui_term_update_status();
}

term_color_pair_t ui_term_color_get() {
    term_color_pair_t tc = { _color_term_text_current_fg, _color_term_text_current_bg };
    return (tc);
}

void ui_term_color_refresh() {
    term_color_bg(_color_term_text_current_bg);
    term_color_fg(_color_term_text_current_fg);
}

void ui_term_color_set(term_color_t fg, term_color_t bg) {
    _color_term_text_current_bg = bg;
    _color_term_text_current_fg = fg;
    term_color_bg(bg);
    term_color_fg(fg);
}

void ui_term_display_speed() {
    const config_t* config = config_current();
    ui_term_update_speed(config->text_speed);
}

void ui_term_display_wire() {
    const config_t* config = config_current();
    ui_term_update_wire(config->wire);
}

static ui_term_control_char_handler ui_term_get_control_char_handler(char c) {
    if (iscntrl(c)) {
        return _control_char_handler[(int)c];
    }
    return (NULL);
}

void ui_term_getline(ui_term_getline_callback_fn getline_cb) {
    _getline_callback = getline_cb;
    ui_term_register_input_available_handler(_ui_term_getline_continue);
    // Use the 'continue' function to process
    _ui_term_getline_continue();
}

void ui_term_getline_cancel(ui_term_input_available_handler input_handler) {
    _getline_callback = NULL;
    ui_term_register_input_available_handler(input_handler);
    _getline_index = 0;
    _getline_buf[_getline_index] = '\0';
}

bool ui_term_handle_control_character(char c) {
    if (iscntrl(c)) {
        ui_term_control_char_handler handler_fn = _control_char_handler[(int)c];
        if (handler_fn) {
            handler_fn(c);
            return (true);
        }
    }
    return (false);
}

extern void ui_term_register_control_char_handler(char c, ui_term_control_char_handler handler_fn) {
    if (iscntrl(c)) {
        _control_char_handler[(int)c] = handler_fn;
    }
}

void ui_term_register_input_available_handler(ui_term_input_available_handler handler_fn) {
    _input_available_handler = handler_fn;
}

void ui_term_update_connected_state(wire_connected_state_t state) {
    char state_char = (WIRE_CONNECTED == state ? UI_TERM_CONNECTED_CHAR : UI_TERM_NOT_CONNECTED_CHAR);

    term_cursor_save();
    term_color_fg(UI_TERM_HEADER_COLOR_FG);
    term_color_bg(UI_TERM_HEADER_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_CONNECTED_ICON_COL);
    printf("[%c]", state_char);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

void ui_term_update_sender(const char* id) {
    char buf[UI_TERM_COLUMNS + 1];

    term_cursor_save();
    term_color_fg(UI_TERM_SENDER_COLOR_FG);
    term_color_bg(UI_TERM_SENDER_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_SENDER_LINE, 1);
    term_erase_line();
    if (id) {
        snprintf(buf, sizeof(buf) - 1, ">%s", id);
        printf("%s", buf);
    }
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

void ui_term_update_speed(uint16_t speed) {
    char buf[UI_TERM_COLUMNS + 1];

    term_cursor_save();
    term_color_fg(UI_TERM_HEADER_COLOR_FG);
    term_color_bg(UI_TERM_HEADER_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_SPEED_VALUE_COL);
    snprintf(buf, sizeof(buf) - 1, "%-2hd", speed);
    printf("%s", buf);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

void ui_term_update_wire(uint16_t wire) {
    char buf[UI_TERM_COLUMNS + 1];

    term_cursor_save();
    term_color_fg(UI_TERM_HEADER_COLOR_FG);
    term_color_bg(UI_TERM_HEADER_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_WIRE_VALUE_COL);
    snprintf(buf, sizeof(buf) - 1, "%-3hd", wire);
    printf("%s", buf);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

void ui_term_update_status() {
    // Put the current time in the center
    char buf[10];
    datetime_t now;

    rtc_get_datetime(&now);
    strdatetime(buf, 9, &now, SDTC_TIME_2CHAR_HOUR | SDTC_TIME_AMPM);
    term_cursor_save();
    term_color_fg(UI_TERM_STATUS_COLOR_FG);
    term_color_bg(UI_TERM_STATUS_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_STATUS_LINE, UI_TERM_STATUS_TIME_COL);
    printf("%s", buf);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

void ui_term_use_code_color() {
    ui_term_color_set(UI_TERM_CODE_COLOR_FG, UI_TERM_CODE_COLOR_BG);
}

void ui_term_use_cmd_color() {
    ui_term_color_set(UI_TERM_CMD_COLOR_FG, UI_TERM_CMD_COLOR_BG);
}