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

#include <stdint.h>
#include "cmt.h"
#include "mkwire.h"
#include "pico/types.h"

/**
 * @brief Build (or rebuild) the UI on the display.
 * @ingroup ui
 */
extern void ui_disp_build(void);

/**
 * @brief Refresh the speed value display in the header.
 * @ingroup ui
 */
extern void ui_disp_display_speed();

/**
 * @brief Refresh the wire number display in the header.
 * @ingroup ui
 */
extern void ui_disp_display_wire();

/**
 * @brief Update the Connected icon based on the state.
 * @ingroup ui
 *
 * @param state The connected state
 */
extern void ui_disp_update_connected_state(wire_connected_state_t state);

/**
 * @brief Update the sender station ID in the top of the display.
 * @ingroup ui
 *
 * @param id The station ID of the sender. A NULL will clear the sender line.
 */
extern void ui_disp_update_sender(const char* id);

/**
 * @brief Update the speed value.
 * @ingroup ui
 *
 * @param speed The speed in WPM
 */
extern void ui_disp_update_speed(uint16_t speed);

/**
 * @brief Update the status bar.
 * @ingroup ui
 */
extern void ui_disp_update_status();

/**
 * @brief Update/refresh the wire number.
 * @ingroup ui
 *
 * @param wire The wire number.
 */
extern void ui_disp_update_wire(uint16_t wire);

#ifdef __cplusplus
}
#endif
#endif // _UI_DISP_H_