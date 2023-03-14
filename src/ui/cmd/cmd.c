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

#define CMD_LINE_MAX_LEN 256
#define CMD_LINE_MAX_ARGS 32

typedef enum _CMD_STATES_ {
    CMD_SNOOZING,   // Waiting for user input to wake us up
    CMD_COLLECTING_LINE,
    CMD_PROCESSING_LINE,
} cmd_state_t;


// Internal declarations

static void _hook_keypress();
static int _skip_to_ws_eol(char* line);


static char _linebuf[CMD_LINE_MAX_LEN];
static int _line_index;
static cmd_state_t _cmd_state = CMD_SNOOZING;

void _notified_of_keypress(void) {
    // Function that was registered. Getting called clears the registration.
    //
    // Post a message to our loop with the character if we can read one (should be able to).
    cmt_msg_t msg;
    char c = term_getc();

    if (c >= 0) {
        msg.id = MSG_CMD_KEY_PRESSED;
        msg.data.c = c;
        postUIMsgBlocking(&msg);
    }
    else {
        // Shouldn't get here since we were notified,
        // but just in case - hook us back
        _hook_keypress();
    }
}

static void _hook_keypress() {
    term_notify_on_input(_notified_of_keypress);
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

static void _process_line() {
    printf("\nCMD - Got command: `%s`\n", _linebuf);
    // go back to Snoozing
    _line_index = 0;
    _cmd_state = CMD_SNOOZING;
    // Hook keypress looking for a ':' to wake us up.
    _hook_keypress();
    // Put the terminal back
    term_cursor_moveto(UI_TERM_CMDLINE, 1);
    term_cursor_on(false);
    term_erase_line();
    term_cursor_restore();
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

void cmd_build_line(cmt_msg_t* msg) {
    // Called when MSG_CMD_KEY_PRESSED is received
    // We should be in a state where we are looking to be woken up
    // or building up a line to process.
    char c = msg->data.c;
    if (CMD_SNOOZING == _cmd_state) {
        // See if the char is 'wakeup'
        if (CMD_WAKEUP_CHAR == c) {
            // Wakeup received, change state to building line.
            _cmd_state = CMD_COLLECTING_LINE;
            // Save the current cursor possition, move it to the bottom, and show it.
            term_cursor_save();
            term_cursor_moveto(UI_TERM_CMDLINE, 1);
            putchar(CMD_PROMPT_CHAR);
            term_cursor_on(true);
            //  See if there is more input.
            if (term_input_available()) {
                c = term_getc();
                // Let the following code continue processing.
            }
            else {
                _hook_keypress(); // Get notified when more characters are available.
                return;
            }
        }
    }
    if (CMD_COLLECTING_LINE == _cmd_state) {
        do {
            if ('\n' == c || '\r' == c) {
                // EOL - Terminate our input line and process
                _linebuf[_line_index] = '\0';
                _cmd_state = CMD_PROCESSING_LINE;
                _line_index = 0;
                _process_line();
                return;
            }
            if (BS == c) {
                // Backspace - move back if we aren't at the BOL
                _linebuf[_line_index] = '\0';
                if (_line_index > 0) {
                    _line_index--;
                    term_cursor_left_1();
                }
            }
            else if (ESC == c) {
                // Escape - go back to Snoozing
                _line_index = 0;
                _cmd_state = CMD_SNOOZING;
                term_cursor_moveto(UI_TERM_CMDLINE, 1);
                term_cursor_on(false);
                term_erase_line();
                term_cursor_restore();
                c = 0;
            }
            else {
                _linebuf[_line_index++] = c;
                putchar(c);
                // See if there are more chars available
                if (_line_index < CMD_LINE_MAX_LEN) {
                    if (term_input_available()) {
                        c = term_getc();
                    }
                    else {
                        c = 0;
                    }
                }
                else {
                    // Beep at them - they need to back up or press enter.
                    putchar('\a');
                    _line_index--;
                    c = 0;
                }
            }
        } while (c > 0);
    }
    // No more input chars are available, but we haven't gotten EOL yet,
    // hook for more, or to wake back up...
    _hook_keypress();
}

void cmd_init() {
    _cmd_state = CMD_SNOOZING;
    _line_index = 0;
    // Hook keypress looking for a ':' to wake us up.
    _hook_keypress();
}
