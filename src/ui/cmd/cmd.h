/**
 * MuKOB CMD Command shell - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _UI_CMD_H_
#define _UI_CMD_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt.h"

#define CMD_WAKEUP_CHAR ':'
#define CMD_PROMPT_CHAR ':'

/**
 * @brief UI Message Loop handler to get a line of input and cause it to be processed.
 * @ingroup ui
 *
 * @param msg The received message
 */
extern void cmd_attn_handler(cmt_msg_t* msg);

/**
 * @brief Initialize the command processor.
 * @ingroup ui
 *
 */
extern void cmd_init(void);


#ifdef __cplusplus
    }
#endif
#endif // _UI_CMD_H_
