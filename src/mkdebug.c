/**
 * MuKOB Debugging flags and utilities.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */
#include "mkdebug.h"
#include "cmt.h"
#include "ui_term.h"
#include "util.h"

volatile uint16_t debugging_flags = 0;
static bool _mk_debug = false;

static int _mkdebug_cmd_debug(int argc, char** argv, const char* unparsed) {
    if (argc > 2) {
        // We only take a single argument.
        cmd_help_display(&cmd_mkdebug_entry, HELP_DISP_USAGE);
        return (-1);
    }
    else if (argc > 1) {
        // Argument is bool (ON/TRUE/YES/1 | <anything-else>) to set flag
        bool b = bool_from_str(argv[1]);
        mk_debug_set(b);
    }
    ui_term_printf("Debug: %s\n", (mk_debug() ? "ON" : "OFF"));

    return (0);
}


const cmd_handler_entry_t cmd_mkdebug_entry = {
    _mkdebug_cmd_debug,
    2,
    ".debug",
    "[ON|OFF]",
    "Set/reset debug flag.",
};


bool mk_debug() {
    return _mk_debug;
}

bool mk_debug_set(bool on) {
    bool temp = _mk_debug;
    _mk_debug = on;
    if (on != temp && cmt_message_loops_running()) {
        cmt_msg_t msg = { MSG_DEBUG_CHANGED };
        msg.data.debug = _mk_debug;
        postBothMsgNoWait(&msg);
    }
    return (temp != on);
}

