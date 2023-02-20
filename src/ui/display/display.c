/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "system_defs.h"
#include "mukboard.h"
#include "display.h"
#include "font.h"
#include "string.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"


int disp_printf(bool paint, const char* format, ...) {
    char buf[1024];
    va_list xArgs;
    va_start(xArgs, format);
    int pl = vsnprintf(buf, sizeof(buf), format, xArgs);
    prints(buf, paint);
    va_end(xArgs);

    return (pl);
}
