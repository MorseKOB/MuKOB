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
#define CMD_CONNECT_TOGGLE_CHAR '\003' // ^C
#define CMD_REINIT_TERM_CHAR '\022' // ^R

#define CMD_PROMPT ':'

typedef enum _CMD_STATES_ {
    CMD_SNOOZING,   // Waiting for user input to wake us up
    CMD_COLLECTING_LINE,
    CMD_PROCESSING_LINE,
    CMD_EXECUTING_COMMAND,
} cmd_state_t;


/**
 * @brief Function prototype for a command.
 * @ingroup ui
 *
 * @param argc The argument count (will be at least 1 - the command as entered).
 * @param argv Pointer to vector (array) of arguments. The value of `argv[0]` is the command).
 *
 * @return Value to pass back to shell.
 */
typedef int (*command_fn)(int argc, char** argv);

/**
 * @brief UI Message Loop handler to get a line of input and cause it to be processed.
 * @ingroup ui
 *
 * @param msg The received message
 */
extern void cmd_attn_handler(cmt_msg_t* msg);

/**
 * @brief Enter the idle state (if not already in it).
 * @ingroup ui
 */
extern void cmd_enter_idle_state();

/**
 * @brief Get the state of the command processor.
 * @ingroup ui
 *
 * @return const cmd_state_t
 */
extern const cmd_state_t cmd_get_state();

/**
 * @brief Initialize the command processor.
 * @ingroup ui
 */
extern void cmd_init(void);

/**
 * @brief Parse the line into arguments (like a 'C' `main` receives)
 *
 * @param line The line to parse. Note that the line will (possibly) be modified.
 * @param argv Pointers to the arguments (indexes into the line).
 * @param maxargs The maximum number of arguments that `argv` can hold.
 * @return int The number of arguments (`argc`).
 */
extern int parse_line(char* line, char** argv, int maxargs);

#ifdef __cplusplus
    }
#endif
#endif // _UI_CMD_H_
