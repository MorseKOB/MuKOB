/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "system_defs.h"
#include "mkboard.h"
#include "display_i.h"
#include "font.h"
#include "string.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"

// Stack of display contexts
#define NUMBER_OF_SCREEN_CONTEXTS 8
static scr_context_t* _scr_contexts[NUMBER_OF_SCREEN_CONTEXTS];
static int _scr_contexts_peek = -1;

bool _has_scr_context() {
    return (_scr_contexts_peek > (-1));
}

scr_context_t* _peek_scr_context() {
    if (_scr_contexts_peek < 0) {
        return NULL;
    }
    return (_scr_contexts[_scr_contexts_peek]);
}

scr_context_t* _pop_scr_context() {
    if (_scr_contexts_peek < 0) {
        error_printf("Display - No screen context to pop.");
        return NULL;
    }
    return (_scr_contexts[_scr_contexts_peek--]);
}

bool _push_scr_context(scr_context_t* sc) {
    if (_scr_contexts_peek < (NUMBER_OF_SCREEN_CONTEXTS - 1)) {
        _scr_contexts[++_scr_contexts_peek] = sc;
        return (true);
    }
    error_printf("Display - Screen context stack is full.");
    return (false);
}


int printf_disp(paint_control_t paint, const char* format, ...) {
    char buf[1024];
    va_list xArgs;
    va_start(xArgs, format);
    int pl = vsnprintf(buf, sizeof(buf), format, xArgs);
    prints(buf, paint);
    va_end(xArgs);

    return (pl);
}
