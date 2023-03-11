/**
 * MuKOB User Interface - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _UI_TERM_H_
#define _UI_TERM_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build (or rebuild) the UI on the terminal.
 * @ingroup ui
 */
extern void ui_term_build(void);

/**
 * @brief Update the sender station ID in the top of the terminal.
 * @ingroup ui
 *
 * @param id The station ID of the sender. A NULL will clear the sender line.
 */
extern void ui_term_sender_update(const char* id);

/**
 * @brief Update the status bar.
 */
extern void ui_term_status_update();


#ifdef __cplusplus
    }
#endif
#endif // _UI_TERM_H_