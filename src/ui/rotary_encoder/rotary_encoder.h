/**
 * @brief rotary encoder decoding functionality.
 * @ingroup ui
 *
 * This provides input from the rotary encoder.
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _rotary_ENQ_H_
#define _rotary_ENQ_H_
#ifdef __cplusplus
extern "C" {
#endif

extern void re_turn_irq_handler(uint gpio, uint32_t events);

/**
 * @brief Initialize the rotary encoder decode library.
 * @ingroup ui
 */
extern void rotary_encoder_module_init();

#ifdef __cplusplus
    }
#endif
#endif // _rotary_ENQ_H
