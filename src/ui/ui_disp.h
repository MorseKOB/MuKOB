/**
 * MuKOB User Interface - On the display, rotary, touch.
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
#include "kob.h"
#include "mkwire.h"
#include "pico/types.h"

/**
 * @brief Build (or rebuild) the UI on the display.
 * @ingroup ui
 */
extern void ui_disp_build(void);

/**
 * @brief Refresh the loop circuit closed indicator in the header.
 * @ingroup ui
 *
 * @param closed True if loop circuit is closed. False if open.
 */
extern void ui_disp_update_circuit_closed(bool closed);

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
 * @brief Print the code-text string. This is 0-n spaces and a character.
 * @ingroup ui
 *
 * @param str The string to print.
 */
void ui_disp_put_codetext(char* str);

/**
 * @brief Print a string in the scrolling (code) area of the screen.
 * @ingroup ui
 *
 * If code is displaying, this will print a newline and then the string.
 *
 * @param str The string to print.
 */
void ui_disp_puts(char* str);

/**
 * @brief Refresh the loop circuit closed indicator in the header.
 * @ingroup ui
 *
 * @param closed True if loop circuit is closed. False if open.
 */
extern void ui_disp_update_circuit_closed(bool closed);

/**
 * @brief Update the Connected icon based on the state.
 * @ingroup ui
 *
 * @param state The connected state
 */
extern void ui_disp_update_connected_state(wire_connected_state_t state);

/**
 * @brief Refresh the key (closer) closed indicator in the header.
 * @ingroup ui
 *
 * @param closed True if key (closer) is closed. False if open.
 */
extern void ui_disp_update_key_closed(bool closed);

/**
 * @brief Update the kob status indicators.
 * @ingroup ui
 *
 * @param kob_status Current KOB status
 */
extern void ui_disp_update_kob_status(const kob_status_t* kob_status);

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
 * @brief Update the active stations list area.
 * @ingroup ui
 *
 * The list is NULL terminated, but we also pass in the count to allow formatting
 * without having to scan the list first.
 *
 * @param stations List of Station ID structure pointers.
 * @param count The number of stations in the list.
 */
extern void ui_disp_update_stations(const mk_station_id_t** stations, int count);

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