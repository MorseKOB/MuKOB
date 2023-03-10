/**
 * MuKOB User Interface - On the display, rotory, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _UI_DISP_H_
#define _UI_DISP_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build (or rebuild) the UI on the display.
 * @ingroup ui
 */
extern void ui_build_disp(void);

/**
 * @brief Update the sender station ID in the top of the display.
 * @ingroup ui
 *
 * @param id The station ID of the sender. A NULL will clear the sender line.
 */
extern void ui_disp_sender_update(const char* id);

/**
 * @brief Update the status bar.
 */
extern void ui_disp_status_update();

#ifdef __cplusplus
}
#endif
#endif // _UI_DISP_H_