/**
 * @brief Rotory encoder decoding functionality.
 * @ingroup ui
 *
 * This provides input from the rotory encoder.
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _ROTORY_ENQ_H_
#define _ROTORY_ENQ_H_
#ifdef __cplusplus
extern "C" {
#endif

extern void re_turn_irq_handler(uint gpio, uint32_t events);

/**
 * @brief Initialize the rotory encoder decode library.
 * @ingroup ui
 */
extern void rotory_encoder_module_init();

#ifdef __cplusplus
    }
#endif
#endif // _ROTORY_ENQ_H
