/**
 * MuKOB User Interface - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _UI_H_
#define _UI_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt.h"

#define UI_CORE_NUM 1

/**
 * @brief Message loop context for use by the loop handler.
 * @ingroup ui
 */
extern msg_loop_cntx_t ui_msg_loop_cntx;

/**
 * @brief Initialize the UI
 */
extern void ui_init(void);

extern void ui_register_input_char_ready_handler();

#ifdef __cplusplus
}
#endif
#endif // _UI_H_
