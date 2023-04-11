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
 * @brief Start the UI (core 1 main and (endless) message-loop).
 */
extern void start_ui(void);

/**
 * @brief Initialize the UI
 */
extern void ui_module_init(void);

#ifdef __cplusplus
}
#endif
#endif // _UI_H_
