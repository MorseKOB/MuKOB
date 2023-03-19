/**
 * MuKOB CMD Command shell - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */

#include "cmd.h"
#include "term.h"
#include "ui_term.h"
#include "util.h"
#include "pico/printf.h"

#include <string.h>

#define CMD_LINE_MAX_ARGS 32

typedef enum _CMD_STATES_ {
    CMD_SNOOZING,   // Waiting for user input to wake us up
    CMD_COLLECTING_LINE,
    CMD_PROCESSING_LINE,
} cmd_state_t;


// Internal declarations

static void _hook_keypress();
static int _skip_to_ws_eol(char* line);


static cmd_state_t _cmd_state = CMD_SNOOZING;
static term_color_pair_t _scr_color_save;
static scr_position_t _scr_cursor_position_save;

void _notified_of_keypress(void) {
    // A character is available from the terminal. Getting called clears the registration.
    //
    // See if it's our wakeup char. If so, post a message to kick off our command processing.
    // If not, throw it away.
    int ci = term_getc();
    while (ci >= 0) {
        char c = (char)ci;
        if (CMD_WAKEUP_CHAR == c) {
            // Post a message to our loop with the character if we can read one (should be able to).
            cmt_msg_t msg;
            msg.id = MSG_CMD_KEY_PRESSED;
            msg.data.c = c;
            postUIMsgBlocking(&msg);
            // We don't re-register, as something else handles the incoming chars until we go idle again.
            return;
        }
        else if (CMD_REINIT_TERM_CHAR == c) {
            // ^R can be typed if the terminal gets messed up or is connected after MuKOB has started.
            // This re-initializes the terminal.
            cmt_msg_t msg;
            msg.id = MSG_CMD_INIT_TERMINAL;
            msg.data.c = c;
            postUIMsgBlocking(&msg);
            // In this case, we do want to re-register to continue being notified of input.
        }
        // If it's not our wake char, read again. When no chars are ready, the call returns -1.
        ci = term_getc();
    }
    // If we get here we need to re-register so we are notified when another character is ready.
    _hook_keypress();
}

static void _hook_keypress() {
    // Look for our wakeup char being sent from the terminal.
    term_register_notify_on_input(_notified_of_keypress);
}

static int _parseline(char* line, char** argv, int maxargs) {
    for (int i = 0; i < maxargs; i++) {
        argv[i] = line; // Store the argument
        int chars_skipped = _skip_to_ws_eol(line);
        // See if this would be the EOL
        if ('\000' == *(line + chars_skipped)) {
            return (i);
        }
        // Store a '\000' for the arg and move to the next
        *(line + chars_skipped) = '\000';
        line = strskipws(line + chars_skipped + 1);
    }
    return (maxargs);
}

static void _process_line(char* line) {
    // Erase the cursor line (where they entered the command)
    term_cursor_bol();
    term_erase_line();

    printf("CMD - Got command: `%s`\n", line);
    // go back to Snoozing
    _cmd_state = CMD_SNOOZING;
    // Hook keypress looking for a ':' to wake us up.
    _hook_keypress();
    // Put the terminal state back
    term_cursor_moveto(UI_TERM_CMDLINE, 1);
    ui_term_color_set(_scr_color_save.fg, _scr_color_save.bg);
    term_cursor_on(false);
    term_erase_line();
    term_cursor_moveto(_scr_cursor_position_save.line, _scr_cursor_position_save.column);
}

static int _skip_to_ws_eol(char* line) {
    int chars_skipped = 0;
    do {
        char c = *(line + chars_skipped);
        if ('\000' == c || '\040' == c || '\n' == c || '\r' == c || '\t' == c) {
            return (chars_skipped);
        }
        chars_skipped++;
    } while (1);
    // shouldn't get here
    return (strlen(line));
}

void cmd_attn_handler(cmt_msg_t* msg) {
    // Called when MSG_CMD_KEY_PRESSED is received
    // We should be in a state where we are looking to be woken up
    // or building up a line to process.
    char c = msg->data.c;
    if (CMD_SNOOZING == _cmd_state) {
        // See if the char is 'wakeup'
        if (CMD_WAKEUP_CHAR == c) {
            // Wakeup received, change state to building line.
            _cmd_state = CMD_COLLECTING_LINE;
            // Get the current cursor possition, move it to the bottom, and show it.
            // NOTE: Updating the Term UI status uses the terminals `save cursor`
            //       capability, so we use the `get cursor position` functionality
            //       and save it.
            _scr_cursor_position_save = term_get_cursor_position();
            _scr_color_save = ui_term_color_get();
            term_cursor_moveto(UI_TERM_CMDLINE, 1);
            ui_term_use_cmd_color();
            putchar(CMD_PROMPT_CHAR);
            term_cursor_on(true);
            // Get a command from the user...
            ui_term_getline(_process_line);
        }
    }
}

void cmd_init() {
    _cmd_state = CMD_SNOOZING;
    // Hook keypress looking for a ':' to wake us up.
    _hook_keypress();
}
