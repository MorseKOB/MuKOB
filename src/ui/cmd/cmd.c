/**
 * MuKOB CMD Command shell - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */

#include "cmd.h"
#include "mkwire.h"
#include "morse.h"
#include "term.h"
#include "ui_term.h"
#include "util.h"
#include "pico/printf.h"

#include <string.h>

#define CMD_LINE_MAX_ARGS 64

// Command processor declarations
static int _cmd_connect(int argc, char** argv);
static int _cmd_encode(int argc, char** argv);
static int _cmd_help(int argc, char** argv);
static int _cmd_wire(int argc, char** argv);

// Command processors framework
typedef struct _CMD_HANDLER_ENTRY {
    command_fn cmd; // Command function to call
    int min_match; // Minimum number of characters to match of the command name.
    char* name;
    char* usage;
    char* description;
} cmd_handler_entry_t;

static cmd_handler_entry_t _cmd_connect_entry = {
    _cmd_connect,
    1,
    "connect",
    "[wire-number]",
    "Connect/disconnect (toggle) the current wire. Connect to a specific wire.",
};
static cmd_handler_entry_t _cmd_encode_entry = {
    _cmd_encode,
    1,
    "encode",
    "<string-to-encode>",
    "Encode a string to Morse.",
};
static cmd_handler_entry_t _cmd_help_entry = {
    _cmd_help,
    1,
    "help",
    "[command_name]",
    "List of commands or information for a specific command.",
};
static cmd_handler_entry_t _cmd_wire_entry = {
    _cmd_wire,
    1,
    "wire",
    "[wire-number]",
    "Display the current wire. Set the wire number.",
};

/**
 * @brief List of Command Handlers
 */
cmd_handler_entry_t* _command_entries[] = {
    &_cmd_connect_entry,
    &_cmd_encode_entry,
    &_cmd_help_entry,
    &_cmd_wire_entry,
    ((cmd_handler_entry_t*)0), // Last entry must be a NULL
};


// Internal (non-command) declarations

typedef enum _CMD_HELP_DISPLAY_FMT_ {
    HELP_DISP_NAME,
    HELP_DISP_LONG,
    HELP_DISP_USAGE,
} cmd_help_display_format_t;

static void _help_display(cmd_handler_entry_t* cmd, cmd_help_display_format_t type);
static void _hook_keypress();
static int _skip_to_ws_eol(char* line);


// Class data

static cmd_state_t _cmd_state = CMD_SNOOZING;
static term_color_pair_t _scr_color_save;
static scr_position_t _scr_cursor_position_save;


// Command functions

static int _cmd_connect(int argc, char** argv) {
    if (argc > 2) {
        _help_display(&_cmd_connect_entry, HELP_DISP_USAGE);
        return (-1);
    }
    int16_t current_wire = mkwire_wire_get();
    if (argc == 2) {
        bool success;
        uint16_t wire = (uint16_t)uint_from_str(argv[1], &success);
        if (!success) {
            printf("Value error - '%s' is not a number.\n", argv[1]);
            return (-1);
        }
        if (wire < 1 || wire > 999) {
            printf("Wire number must be 1 to 999.\n");
            return (-1);
        }
        if (wire != current_wire) {
            // The wire specified is not the current wire - connect to it.
            printf("Connecting to wire %hu...\n", wire);
            cmt_msg_t msg;
            msg.id = MSG_WIRE_CONNECT;
            msg.data.wire = wire;
            postBEMsgBlocking(&msg);
            return (0);
        }
    }
    char* op = (mkwire_is_connected() ? "Disconnecting from" : "Connecting to");
    printf("%s wire %hu...\n", op, current_wire);
    cmt_msg_t msg;
    msg.id = MSG_WIRE_CONNECT_TOGGLE;
    postBEMsgBlocking(&msg);
    return (0);
}

static int _cmd_encode(int argc, char** argv) {
    if (argc < 2) {
        _help_display(&_cmd_encode_entry, HELP_DISP_USAGE);
        return (-1);
    }
    cmt_msg_t msg;
    mcode_t* mcode_space;
    for (int i = 1; i < argc; i++) {
        char* str = argv[i];
        char c;
        while ('\000' != (c = *str++)) {
            mcode_t* mcode = morse_encode(c);
            // Post it to the backend to decode
            msg.id = MSG_MORSE_TO_DECODE;
            msg.data.mcode = mcode;
            postBEMsgBlocking(&msg);
        }
        if (i+1 < argc) {
            // Add a space
            mcode_space = morse_encode(' ');
            msg.id = MSG_MORSE_TO_DECODE;
            msg.data.mcode = mcode_space;
            postBEMsgBlocking(&msg);
        }
    }

    return (0);
}

static int _cmd_help(int argc, char** argv) {
    cmd_handler_entry_t** cmds = _command_entries;
    cmd_handler_entry_t* cmd;
    if (1 == argc) {
        // List all of the commands with thier usage.
        printf("Commands:\n");
        while (NULL != (cmd = *cmds++)) {
            _help_display(cmd, HELP_DISP_NAME);
        }
    }
    else {
        // They entered command names
        char* user_cmd;
        for (int i = 1; i < argc; i++) {
            cmds = _command_entries;
            user_cmd = argv[i];
            int user_cmd_len = strlen(user_cmd);
            bool command_matched = false;
            while (NULL != (cmd = *cmds++)) {
                int cmd_name_len = strlen(cmd->name);
                if (user_cmd_len <= cmd_name_len && user_cmd_len >= cmd->min_match) {
                    if (0 == strncmp(cmd->name, user_cmd, user_cmd_len)) {
                        // This command matches
                        command_matched = true;
                        _help_display(cmd, HELP_DISP_LONG);
                        break;
                    }
                }
            }
            if (!command_matched) {
                printf("Unknown: '%s'\n", user_cmd);
            }
        }
    }

    return (0);
}

static int _cmd_wire(int argc, char** argv) {
    if (argc > 2) {
        _help_display(&_cmd_wire_entry, HELP_DISP_USAGE);
        return (-1);
    }
    if (argc == 2) {
        bool success;
        uint16_t wire = (uint16_t)uint_from_str(argv[1], &success);
        if (!success) {
            printf("Value error - '%s' is not a number.\n", argv[1]);
            return (-1);
        }
        if (wire < 1 || wire > 999) {
            printf("Wire number must be 1 to 999.\n");
            return (-1);
        }
        cmt_msg_t msg;
        msg.id = MSG_WIRE_SET;
        msg.data.wire = wire;
        postBEMsgBlocking(&msg);
    }
    else {
        printf("%hd\n", mkwire_wire_get());
    }
    return (0);
}

// Internal functions

/**
 * @brief Registered to handle ^C to toggle the connect/disconnect.
 * @ingroup ui
 *
 * @param c Should be ^C
 */
void _handle_connect_toggle_char(char c) {
    // ^C can be typed to connect/disconnect.
    cmt_msg_t msg;
    msg.id = MSG_WIRE_CONNECT_TOGGLE;
    postBEMsgBlocking(&msg);
}

/**
 * @brief Registered to handle ^R to re-initialize the terminal.
 *
 * @param c Should be ^R
 */
void _handle_reinit_terminal_char(char c) {
    // ^R can be typed if the terminal gets messed up or is connected after MuKOB has started.
    // This re-initializes the terminal.
    cmt_msg_t msg;
    msg.id = MSG_CMD_INIT_TERMINAL;
    msg.data.c = c;
    postUIMsgBlocking(&msg);
}

static void _help_display(cmd_handler_entry_t* cmd, cmd_help_display_format_t type) {
    term_color_pair_t tc = ui_term_color_get();
    term_color_default();
    if (HELP_DISP_USAGE == type) {
        printf("Usage: ");
    }
    int name_min = cmd->min_match;
    char* name_rest = ((cmd->name) + name_min);
    char format[16];
    // Print the minimum required characters bold and the rest normal.
    snprintf(format, 16, " %%.%ds", name_min);
    term_text_bold();
    printf(format, cmd->name);
    term_text_normal();
    printf("%s %s\n", name_rest, cmd->usage);
    if (HELP_DISP_LONG == type || HELP_DISP_USAGE == type) {
        printf("     %s\n", cmd->description);
    }
    term_color_fg(tc.fg);
    term_color_bg(tc.bg);
}

void _notified_of_keypress() {
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
        else {
            ui_term_handle_control_character(c); // This might not be a control char, but that's okay.
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

static void _process_line(char* line) {
    char* argv[CMD_LINE_MAX_ARGS];
    memset(argv, 0, sizeof(argv));

    _cmd_state = CMD_PROCESSING_LINE;

    printf("\n");

    int argc = parse_line(line, argv, CMD_LINE_MAX_ARGS);
    char* user_cmd = argv[0];
    int user_cmd_len = strlen(user_cmd);
    bool command_matched = false;

    if (user_cmd_len > 0) {
        cmd_handler_entry_t** cmds = _command_entries;
        cmd_handler_entry_t* cmd;

        while (NULL != (cmd = *cmds++)) {
            int cmd_name_len = strlen(cmd->name);
            if (user_cmd_len <= cmd_name_len && user_cmd_len >= cmd->min_match) {
                if (0 == strncmp(cmd->name, user_cmd, user_cmd_len)) {
                    // This command matches
                    command_matched = true;
                    _cmd_state = CMD_EXECUTING_COMMAND;
                    cmd->cmd(argc, argv);
                    break;
                }
            }
        }
        if (!command_matched) {
            printf("Command not found: '%s'. Try 'help'.\n", user_cmd);
        }
    }

    // If we aren't currently connected, get another line from the user.
    if (!mkwire_is_connected()) {
        // Get a command from the user...
        _cmd_state = CMD_COLLECTING_LINE;
        printf("%c", CMD_PROMPT);
        ui_term_getline(_process_line);
    }
    else {
        cmd_enter_idle_state();
    }
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


// Public functions

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
            // Get the current cursor position, move it to the bottom, and show it.
            // NOTE: Updating the Term UI status uses the terminals `save cursor`
            //       capability, so we use the `get cursor position` functionality
            //       and save it.
            _scr_cursor_position_save = term_get_cursor_position();
            _scr_color_save = ui_term_color_get();
            term_cursor_moveto(UI_TERM_CMDLINE, 1);
            ui_term_use_cmd_color();
            putchar(CMD_PROMPT);
            term_cursor_on(true);
            // Get a command from the user...
            ui_term_getline(_process_line);
        }
    }
}

/**
 * This is typically called by the UI when the system connects to a wire.
 */
extern void cmd_enter_idle_state() {
    if (CMD_SNOOZING != _cmd_state) {
        // Cancel any inprocess 'getline'
        ui_term_getline_cancel(_notified_of_keypress);
        // Put the terminal state back
        term_cursor_moveto(UI_TERM_CMDLINE, 1);
        ui_term_color_set(_scr_color_save.fg, _scr_color_save.bg);
        term_cursor_on(false);
        term_erase_line();
        term_cursor_moveto(_scr_cursor_position_save.line, _scr_cursor_position_save.column);

        // go back to Snoozing
        _cmd_state = CMD_SNOOZING;
    }
}

const cmd_state_t cmd_get_state() {
    return _cmd_state;
}

void cmd_init() {
    _cmd_state = CMD_SNOOZING;
    // Register the control character handlers.
    ui_term_register_control_char_handler(CMD_CONNECT_TOGGLE_CHAR, _handle_connect_toggle_char);
    ui_term_register_control_char_handler(CMD_REINIT_TERM_CHAR, _handle_reinit_terminal_char);
    // Hook keypress looking for a ':' to wake us up.
    _hook_keypress();
}

int parse_line(char* line, char** argv, int maxargs) {
    for (int i = 0; i < maxargs; i++) {
        argv[i] = line; // Store the argument
        int chars_skipped = _skip_to_ws_eol(line);
        // See if this would be the EOL
        if ('\000' == *(line + chars_skipped)) {
            return (i + 1);
        }
        // Store a '\000' for the arg and move to the next
        *(line + chars_skipped) = '\000';
        line = strskipws(line + chars_skipped + 1);
    }
    return (maxargs);
}
