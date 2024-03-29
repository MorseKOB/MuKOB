/**
 * MorseKOB Wire (network) functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _MUKOB_MKWIRE_H_
#define _MUKOB_MKWIRE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt.h"

#include <stdbool.h>
#include "pico/types.h"

#define MKOBSERVER_DEFAULT "mtc-kob.dyndns.org"
#define MKOBSERVER_PORT_DEFAULT 7890
#define MKOBSERVER_STATION_ID_MAX_LEN 127

typedef enum _WIRE_CONNECTED_STATE_ {
    WIRE_NOT_CONNECTED,
    WIRE_CONNECTED,
} wire_connected_state_t;

/**
 * @brief Structure for an active MorseKOB station.
 * @ingroup wire
 *
 * @param id Name of the station
 * @param ts_init millisecond timestamp when the station first connected
 * @param ts_ping millisecond timestamp of the last ping from this station
 * @param ts_recv millisecond timestamp of the last receipt from this station
 */
typedef struct _station_id_ {
    char id[MKS_PKT_MAX_STRING_LEN + 1];
    uint32_t ts_init;
    uint32_t ts_ping;
    uint32_t ts_recv;
} mk_station_id_t;
#define MK_MAX_ACTIVE_STATIONS 32


/**
 * @brief Get a list of the active stations.
 * @ingroup wire
 *
 * @return const mk_station_id_t** List of pointers to station ID structures.
 */
extern const mk_station_id_t** mkwire_active_stations();

/*!
 * @brief Disconnect from the currently connected MorseKOB Wire.
 * @ingroup wire
 */
extern void mkwire_disconnect();

/*!
 * @brief Connect to a MorseKOB Wire.
 * @ingroup wire
 *
 * @param wire_no The wire number to connect to (1-999)
 */
extern void mkwire_connect(unsigned short wire_no);

/*!
 * @brief Toggle connection (Connect or Disconnect) to a MorseKOB Wire.
 * @ingroup wire
 *
 * Toggle connection is a common operation. This requires that a wire
 * has been set.
 */
extern void mkwire_connect_toggle();

/**
 * @brief The wire connected state.
 * @ingroup wire
 *
 * @return wire_connected_state_t The current state
 */
extern wire_connected_state_t mkwire_connected_state();

/**
 * @brief The current sender or NULL.
 * @ingroup wire
 *
 * @return const char*
 */
extern const char* mkwire_current_sender();

/*!
 * @brief Initialize the MorseKOB Wire subsystem.
 * @ingroup wire
 *
 * @param mkobs_url MorseKOB Server URL.
 * @param office_id The local Station/Office ID.
 * @param wire_no The wire number to use for connect.
 */
extern void mkwire_module_init(char* mkobs_url, uint16_t port, char* office_id, uint16_t wire_no);

/*!
 * @brief Connected to KOB Server status.
 * @ingroup wire
 *
 * @returns True if currently connected to a KOB Server wire.
 */
extern bool mkwire_is_connected();

/**
 * @brief Send the station ID to the MorseKOB Server if currently connected.
 * @ingroup wire
 *
 */
extern void mkwire_keep_alive_send();

/**
 * @brief Handle a message containing a pbuf packet received from a Morse KOB Server.
 *
 * @param msg Message containing a pbuf received from a Morse KOB Server.
 */
extern void mkwire_handle_packet_received(cmt_msg_t* msg);

/*!
 * @brief Set the local Station/Office ID.
 * @ingroup wire
 *
 * @param office_id The local Station/Office ID.
 */
extern void mkwire_set_office_id(char* office_id);

/**
 * @brief Get the current wire number.
 * @ingroup wire
 *
 * @return int16_t The wire number.
 */
extern uint16_t mkwire_wire_get();

/**
 * @brief Set the wire number (without connecting).
 * @ingroup wire
 *
 * This can also be done with the `mkwire_connect` function,
 * but clients sometimes want to set the wire without connecting.
 *
 * @param wire_no Wire number 1-999 are used/allowed by MorseKOB Server.
 */
extern void mkwire_wire_set(uint16_t wire_no);

#ifdef __cplusplus
}
#endif
#endif // _MUKOB_MKWIRE_H_
