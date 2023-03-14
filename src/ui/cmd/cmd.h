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
 * @brief UI Message Loop handler to build up a line of input as characters are typed.
 * @ingroup ui
 *
 * Being message based, this is called when our CMD message is received. When a full
 * line is built up (a '\n' or '\r' is received) we call the line processing.
 *
 * @param msg The received message
 */
extern void cmd_build_line(cmt_msg_t* msg);

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
