/**
 * @brief rotary encoder push-button switch functionality.
 * @ingroup ui
 *
 * This provides input from the rotary encoder push-button switch.
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _RE_PBSW_H_
#define _RE_PBSW_H_
#ifdef __cplusplus
extern "C" {
#endif

extern void re_pbsw_irq_handler(uint gpio, uint32_t events);

/**
 * @brief Initialize the rotary encoder push-button switch library.
 * @ingroup ui
 */
extern void re_pbsw_module_init();

#ifdef __cplusplus
}
#endif
#endif // _RE_PBSW_H_
