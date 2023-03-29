/**
 * MuKOB Debugging flags and utilities.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 */
#ifndef _DEBUGGING_H_
#define _DEBUGGING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

extern volatile uint16_t debugging_flags;
#define DEBUGGING_MORSE_DECODE 0x0001
#define DEBUGGING_MORSE_DECODE_SKIP 0x0002

#ifdef __cplusplus
}
#endif
#endif // _DEBUGGING_H_
