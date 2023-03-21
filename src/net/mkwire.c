/**
 * MorseKOB Wire (network) functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#include <stddef.h>
#include <string.h>

#include "mkwire.h"
#include "config.h"
#include "cmt.h"
#include "net.h"
#include "mkboard.h"

#define ADDR_PORT_SEP ':'

/*
 * *** Message format To/From MorseKOB Server ***

 * From PyKOB (https://github.com/MorseKOB/PyKOB)
 *
 * shortPacketFormat = struct.Struct("<hh")  # cmd, wire
 * idPacketFormat = struct.Struct("<hh 128s 4x i i 8x 208x 128s 8x")  # cmd, byts, id, seq, idflag, ver
 * codePacketFormat = struct.Struct("<hh 128s 4x i 12x 51i i 128s 8x")  # cmd, byts, id, seq, code list, n, txt
 *
 * Python Struct format:
 * Format:
 * '<' indicates little-endian byte order
 * ----+-----------+---------------+-----------+
 * C   | Type      | Python type   | C Size    |
 * ----+-----------+---------------+-----------+
 * x   | pad byte  | no value      |           |
 * ----+-----------+---------------+-----------+
 * h   | short     | integer       | 2         |
 * ----+-----------+---------------+-----------+
 * i   | int       | integer       | 4         |
 * ----+-----------+---------------+-----------+
 * s   | char[]    | bytes (string)|           |
 * ----+-----------+---------------+-----------+
 * 'x' inserts one NUL byte
 */

#define MKS_CMD_DISCONNECT    2  // Disconnect
#define MKS_CMD_DATA          3  // Code or ID
#define MKS_CMD_CONNECT       4  // Connect
#define MKS_CMD_ACK           5  // Ack

#define MKS_CODE_PKT_SIZE 492 // This is from the beginning of the ID to the end, not the actual size
#define MKS_ID_PKT_SIZE 492 // This is from the beginning of the ID to the end, not the actual size
#define MKS_ID_FLAG 1  // Flag indicating that message data is an ID
#define MKS_PKT_MAX_CODE_LEN 51
#define MKS_PKT_MAX_STRING_LEN 127  // Leave room for a '\0' terminator in a 128 byte field

#define MuKOB_VERSION_INFO "MuKOB v0.1"  // ZZZ get from a central name/version string
#define MKS_OP_NOTIMEOUT 0
#define MKS_OP_TIMEOUT (15 * 1000)     // 15 second request/response timeout
#define MKS_KEEP_ALIVE_TIME (20 * 1000) // Time period to send ID to keep us connected

// *** Local function declarations...
void _bind_handler(err_enum_t status, struct udp_pcb* udp_pcb);
static struct pbuf* _connect_req_builder();
static struct pbuf* _disconnect_req_builder();
static struct pbuf* _send_id_req_builder();
static void _mks_recv(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* addr, u16_t port);
static void _send_id();
static void _start_keep_alive();
static void _stop_keep_alive();
static void _wire_connect();

const char* _mks_commands[6] = {
    "*UNDEFINED*",
    "*UNDEFINED*",
    "DISCONNECT",
    "CODE/ID",
    "CONNECT",
    "ACK"
};
#define MAX_VALID_CMD 5

/**
 * @brief shortPacketFormat
 * @ingroup wire
 *
 * ("<hh")  # cmd, wire
 */
typedef struct mkspkt_cmd_wire {
    int16_t cmd;        // h (2)
    int16_t wire;       // h (2)
} mkspkt_cmd_wire_t;
// Cmd+Wire Packet Member offsets:
#define MKSPKT_CW_OFFSET_CMD 0
#define MKSPKT_CW_OFFSET_WIRE 2

/**
 * @brief idPacketFormat
 * @ingroup wire
 *
 * ("<hh 128s 4x i i 8x 208x 128s 8x")  # cmd, byts, id, seq, idflag, ver
 */
typedef struct mkspkt_id {
    int16_t cmd;                                // h (2)
    int16_t bytes;                              // h (2) = size from `id` to the end (492)
    char id[MKS_PKT_MAX_STRING_LEN + 1];        // 128s
    uint8_t pad1[4];                            // 4x
    int32_t seqno;                              // i (4)
    int32_t idflag;                             // i (4)
    uint8_t pad2[8];                            // 8x
    uint8_t pad3[208];                          // 208x
    char version[MKS_PKT_MAX_STRING_LEN + 1];   // 128s
    uint8_t pad4[8];                            // 8x
    //                                          ==========
    //                                           496 bytes
} mkspkt_id_t;
// ID Packet Member offsets:
#define MKSPKT_ID_OFFSET_CMD 0
#define MKSPKT_ID_OFFSET_BYTES 2
#define MKSPKT_ID_OFFSET_ID 4
#define MKSPKT_ID_OFFSET_PAD1 132
#define MKSPKT_ID_OFFSET_SEQNO 136
#define MKSPKT_ID_OFFSET_IDFLAG 140
#define MKSPKT_ID_OFFSET_PAD2 144
#define MKSPKT_ID_OFFSET_PAD3 152
#define MKSPKT_ID_OFFSET_VERSION 360
#define MKSPKT_ID_OFFSET_PAD4 488
#define MKSPKT_ID_LEN 496

/**
 * @brief codePacketFormat
 * @ingroup wire
 *
 * ("<hh 128s 4x i 12x 51i i 128s 8x")  # cmd, byts, id, seq, code list, n, txt
 */
typedef struct mkspkt_code {
    int16_t cmd;                                // h (2)
    int16_t bytes;                              // h (2) = size from `id` to the end (492)
    char id[MKS_PKT_MAX_STRING_LEN + 1];        // 128s
    uint8_t pad1[4];                            // 4x
    int32_t seqno;                              // i (4)
    uint8_t pad2[12];                           // 12x
    int32_t code_list[MKS_PKT_MAX_CODE_LEN];    // 51i (204)
    int32_t n;                                  // i (4)
    char text[MKS_PKT_MAX_STRING_LEN + 1];      // 128s
    uint8_t pad3[8];                            // 8x
    //                                          ==========
    //                                           496 bytes
} mkspkt_code_t;
// Code Packet Member offsets:
#define MKSPKT_CODE_OFFSET_CMD 0
#define MKSPKT_CODE_OFFSET_BYTES 2
#define MKSPKT_CODE_OFFSET_ID 4
#define MKSPKT_CODE_OFFSET_PAD1 132
#define MKSPKT_CODE_OFFSET_SEQNO 136
#define MKSPKT_CODE_OFFSET_PAD2 140
#define MKSPKT_CODE_OFFSET_CODE_LIST 152
#define MKSPKT_CODE_OFFSET_N 356
#define MKSPKT_CODE_OFFSET_TEXT 360
#define MKSPKT_CODE_OFFSET_PAD3 488
#define MKSPKT_CODE_LEN 496

static char *_office_id = NULL;
static char* _mkserver_host = NULL;
static uint16_t _mkserver_port = 0;
static uint16_t _wire_no = 1;
static int32_t _seqno_send = 0;
static int32_t _seqno_recv = -1;
static struct udp_pcb* _udp_pcb = NULL;
static wire_connected_state_t _connected_state = WIRE_NOT_CONNECTED;
static struct repeating_timer _keep_alive_timer;
static bool _keep_alive_active = false;
static void (*_next_fn)(void) = NULL;

void mkwire_disconnect() {
    if (_udp_pcb) {
        // Send a disconnect message and free up the UDP connection.
        struct pbuf* p = _disconnect_req_builder();
        err_enum_t status = udp_send(_udp_pcb, p);
        debug_printf("MKS disconnect response status: %d\n", status);
        pbuf_free(p);
        udp_remove(_udp_pcb);
        _udp_pcb = NULL;
        _connected_state = WIRE_NOT_CONNECTED;
    }
    // Post a message to the UI letting it know we are connected
    cmt_msg_t msg;
    msg.id = MSG_WIRE_CONNECTED_STATE;
    msg.data.status = _connected_state;
    postUIMsgBlocking(&msg);
    _stop_keep_alive();
}

void mkwire_connect(unsigned short wire_no) {
    if (mkwire_is_connected()) {
        mkwire_disconnect();
    }
    mkwire_wire_set(wire_no);
    _wire_connect();
}

void mkwire_connect_toggle() {
    if (mkwire_is_connected()) {
        mkwire_disconnect();
    }
    else {
        _wire_connect();
    }
}

wire_connected_state_t mkwire_connected_state() {
    return (_connected_state);
}

void mkwire_init(char* mkobs_url, uint16_t port, char* office_id, uint16_t wire_no) {
    if (_mkserver_host) {
        free(_mkserver_host);
        _mkserver_host = NULL;
    }
    _mkserver_host = malloc(strlen(mkobs_url) + 1);
    if (!_mkserver_host) {
        error_printf("malloc failed in `mkwire_init`\n");
        return;
    }
    strcpy(_mkserver_host, mkobs_url);
    _mkserver_port = port;
    mkwire_set_office_id(office_id);
    mkwire_wire_set(wire_no);
}

bool mkwire_is_connected() {
    return (_udp_pcb != NULL);
}

bool mkwire_host_from_hostport(char* buf, uint32_t maxlen, const char* host_and_port) {
    const char* h2u = (host_and_port ? host_and_port : MKOBSERVER_DEFAULT);
    int len_of_host;

    // See if there is a ':'
    char* sep = strchr(h2u, ADDR_PORT_SEP);
    if (sep) {
        // There is a ':', get the string up to it
        len_of_host = sep - h2u;
    }
    else {
        len_of_host = strlen(h2u);
    }
    if (maxlen > len_of_host) {
        strncpy(buf, h2u, len_of_host);
    }
    else {
        strncpy(buf, h2u, maxlen);
    }

    return (len_of_host < maxlen);
}

uint16_t mkwire_port_from_hostport(const char* host_and_port) {
    // Is there a ':'?
    char* sep = strchr(host_and_port, ADDR_PORT_SEP);
    if (NULL == sep) {
        return MKOBSERVER_PORT_DEFAULT;
    }
    return (atoi(sep+1));
}

void mkwire_set_office_id(char* office_id) {
    if (_office_id) {
        free(_office_id);
        _office_id = NULL;
    }
    _office_id = malloc(strlen(office_id) + 1);
    if (!_office_id) {
        error_printf("malloc failed in mkwire_set_office_id\n");
    }
    strcpy(_office_id, office_id);
}

uint16_t mkwire_wire_get() {
    return (_wire_no);
}

void mkwire_wire_set(uint16_t wire_no) {
    if (wire_no > 0 && wire_no < 1000) {
        _wire_no = wire_no;
        config_t* cfg = config_current_for_modification();
        cfg->wire = wire_no;
        // If we are currently connected, disconnect and re-connect with the new wire.
        if (mkwire_is_connected()) {
            mkwire_disconnect();
            _wire_connect();
        }
        cmt_msg_t msg;
        msg.id = MSG_WIRE_CHANGED;
        msg.data.wire = wire_no;
        postUIMsgBlocking(&msg);
    }
}

// *** Local functions ***

/*!
 * @brief Called by `udp_socket_bind` when the UDP socket has been bound locally
 * and connected to the MKServer.
 */
void _bind_handler(err_enum_t status, struct udp_pcb* udp_pcb) {
    if (status == ERR_OK) {
        _udp_pcb = udp_pcb;
        _connected_state = WIRE_CONNECTED;
        // Set up to receive incoming messages from the MKServer.
        udp_recv(udp_pcb, _mks_recv, NULL);  // Can pass user-data in 3rd arg if needed
        _send_id();
        // Post a message to the UI letting it know we are connected
        cmt_msg_t msg;
        msg.id = MSG_WIRE_CONNECTED_STATE;
        msg.data.status = _connected_state;
        postUIMsgBlocking(&msg);
    }
}

/**
 * @brief Pack an mkspkt_code structure (Code Packet) with values
 * @ingroup wire
 *
 * PyKOB: codePacketFormat.pack(DAT, 492, self.officeID.encode('latin-1'), self.sentSeqNo, *codeBuf)
 *
 * @param code_pkt The Code Packet stucture to set values into
 * @param code Array of int code values
 * @param n Number of code values
 */
static void _pack_code_packet(mkspkt_code_t* code_pkt, int32_t code[], int n) {
    // start with all zeros
    memset(code_pkt, 0x00, sizeof(mkspkt_id_t));
    code_pkt->cmd = MKS_CMD_DATA;
    code_pkt->bytes = MKS_CODE_PKT_SIZE;
    strncpy(code_pkt->id, _office_id, MKS_PKT_MAX_STRING_LEN);
    code_pkt->seqno = _seqno_send;
    code_pkt->n = n;
    memcpy(code_pkt->code_list, code, sizeof(int32_t) * n);
}

/**
 * @brief Pack an mkspkt_id structure (ID Packet) with values
 * @ingroup wire
 *
 * PyKOB: idPacketFormat.pack(DAT, 492, self.officeID.encode('latin-1'), self.sentSeqNo, 1, self.version)
 *
 * @param id_pkt The ID Packet structure to set values into
 */
static void _pack_id_packet(mkspkt_id_t *id_pkt) {
    // start with all zeros
    memset(id_pkt, 0x00, sizeof(mkspkt_id_t));
    id_pkt->cmd = MKS_CMD_DATA;
    id_pkt->bytes = MKS_ID_PKT_SIZE;
    strncpy(id_pkt->id, _office_id, MKS_PKT_MAX_STRING_LEN);
    id_pkt->seqno = _seqno_send;
    id_pkt->idflag = MKS_ID_FLAG;
    strncpy(id_pkt->version, MuKOB_VERSION_INFO, MKS_PKT_MAX_STRING_LEN);
}

/**
 * @brief Unpack a received Code message into an mkspkt_code structure (Code Packet)
 * @ingroup wire
 *
 * PyKOB: cp = .unpack(buf)codePacketFormat
 *        cmd, byts, stnID, seqNo, code = cp[0], cp[1], cp[2], cp[3], cp[4:]
 *        stnID, sep, fill = stnID.decode(encoding='latin-1').partition(NUL)
 *        n = code[51]
 * ("<hh 128s 4x i 12x 51i i 128s 8x")  # cmd, byts, id, seq, code list, n, txt
 *
 * @param code_pkt The Code Packet structure to set with data (the pads are ignored (not filled))
 * @param p The PBUF received from the MorseKOB Server (caller to assure it's not NULL)
 *          The caller is responsible for freeing it (they might have more they want to do with it)
 */
static void _unpack_code_packet(mkspkt_code_t* code_pkt, const struct pbuf* p) {
    memset(code_pkt, 0, sizeof(mkspkt_code_t));
    pbuf_copy_partial(p, &code_pkt->cmd, sizeof(member_size(mkspkt_code_t, cmd)), MKSPKT_CODE_OFFSET_CMD);
    pbuf_copy_partial(p, &code_pkt->bytes, sizeof(member_size(mkspkt_code_t, bytes)), MKSPKT_CODE_OFFSET_BYTES);
    pbuf_copy_partial(p, &code_pkt->id, MKS_PKT_MAX_STRING_LEN, MKSPKT_CODE_OFFSET_ID);
    pbuf_copy_partial(p, &code_pkt->seqno, sizeof(member_size(mkspkt_code_t, seqno)), MKSPKT_CODE_OFFSET_SEQNO);
    pbuf_copy_partial(p, &code_pkt->code_list, sizeof(int32_t) * MKS_PKT_MAX_CODE_LEN, MKSPKT_CODE_OFFSET_CODE_LIST);
    pbuf_copy_partial(p, &code_pkt->n, sizeof(member_size(mkspkt_code_t, n)), MKSPKT_CODE_OFFSET_N);
    pbuf_copy_partial(p, &code_pkt->text, MKS_PKT_MAX_STRING_LEN, MKSPKT_CODE_OFFSET_TEXT);
}

/**
 * @brief Unpack a received ID message into an mkspkt_id structure (ID Packet)
 * @ingroup wire
 *
 * PyKOB: *Doesn't unpack an ID message. It unpacks code and treats it as an ID if
 *         the code length (n) is 0.
 * ("<hh 128s 4x i i 8x 208x 128s 8x")  # cmd, byts, id, seq, idflag, ver
 *
 * @param id_pkt The id_pkt Packet structure to set with data (the pads are ignored (not filled))
 * @param p The PBUF received from the MorseKOB Server (caller to assure it's not NULL)
 *          The caller is responsible for freeing it (they might have more they want to do with it)
 */
static void _unpack_id_packet(mkspkt_id_t* id_pkt, const struct pbuf* p) {
    memset(id_pkt, 0, sizeof(mkspkt_id_t));
    pbuf_copy_partial(p, &id_pkt->cmd, sizeof(member_size(mkspkt_id_t, cmd)), MKSPKT_ID_OFFSET_CMD);
    pbuf_copy_partial(p, &id_pkt->bytes, sizeof(member_size(mkspkt_id_t, bytes)), MKSPKT_ID_OFFSET_BYTES);
    pbuf_copy_partial(p, &id_pkt->id, MKS_PKT_MAX_STRING_LEN, MKSPKT_ID_OFFSET_ID);
    pbuf_copy_partial(p, &id_pkt->seqno, sizeof(member_size(mkspkt_id_t, seqno)), MKSPKT_ID_OFFSET_SEQNO);
    pbuf_copy_partial(p, &id_pkt->idflag, sizeof(member_size(mkspkt_id_t, idflag)), MKSPKT_ID_OFFSET_IDFLAG);
    pbuf_copy_partial(p, &id_pkt->version, MKS_PKT_MAX_STRING_LEN, MKSPKT_ID_OFFSET_VERSION);
}

static struct pbuf* _connect_req_builder() {
    debug_printf("Make MKS `connect` request\n");

    mkspkt_cmd_wire_t connect_packet = { MKS_CMD_CONNECT, (int16_t)_wire_no };
    size_t msg_len = sizeof(mkspkt_cmd_wire_t);
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, msg_len, PBUF_RAM);
    if (p) {
        uint8_t* req = (uint8_t*)p->payload;
        memcpy(req, &connect_packet, msg_len);
    }

    return (p);
}

static struct pbuf* _disconnect_req_builder() {
    debug_printf("Make MKS `disconnect` request\n");

    mkspkt_cmd_wire_t disconnect_packet = { MKS_CMD_DISCONNECT, 0 };
    size_t msg_len = sizeof(mkspkt_cmd_wire_t);
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, msg_len, PBUF_RAM);
    if (p) {
        uint8_t* req = (uint8_t*)p->payload;
        memcpy(req, &disconnect_packet, msg_len);
    }

    return (p);
}

static struct pbuf* _send_id_req_builder() {
    debug_printf("Make MKS `send id` request\n");

    mkspkt_id_t id_packet;
    _pack_id_packet(&id_packet);
    size_t msg_len = sizeof(mkspkt_id_t);
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, msg_len, PBUF_RAM);
    if (p) {
        uint8_t* req = (uint8_t*)p->payload;
        memcpy(req, &id_packet, msg_len);
    }

    return (p);
}

static void _mks_recv(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* ip_addr, u16_t port) {
    if (p != NULL) {
        int16_t cmd;
        pbuf_copy_partial(p, &cmd, sizeof(int16_t), 0);
        uint16_t total_len = p->tot_len;
        if (cmd > MAX_VALID_CMD) {
            error_printf("MKOB Server sent invalid command: %hi Message len: %hu\n", cmd, total_len);
        }
        else {
            const char* cmds = _mks_commands[cmd];
            debug_printf("MKOB Server response received. CMD: %hi (%s) Message len: %hu\n", cmd, cmds, total_len);
            if (MKS_CMD_ACK == cmd) {
                // ACK received, see if there is a followup function to call (send_id -> send_id2)...
                if (_next_fn != NULL) {
                    // Call our next function...
                    void (*fn)(void) = _next_fn;
                    _next_fn = NULL;
                    (*fn)();
                }
            }
            else if (MKS_CMD_DATA) {
                // Data or ID
                // Unpack as Data first, if 'n' is zero, then it is an ID
                mkspkt_code_t code_pkt;
                _unpack_code_packet(&code_pkt, p);
                if (code_pkt.n > 0) {
                    // It is a Code packet, process it...
                    // ZZZ for now, just print it
                    debug_printf("MKS Code: Bytes:%hi SeqNo:%i n:%i ID:'%s' Text:'%s' Code:", 
                        code_pkt.bytes, code_pkt.seqno, code_pkt.n, code_pkt.id, code_pkt.text);
                    for (int i = 0; i < code_pkt.n; i++) {
                        printf("%i ", code_pkt.code_list[i]);
                    }
                    printf("\n");
                    //led_blink_mcode(code_pkt.code_list, code_pkt.n);
                }
                else {
                    // It is an ID packet, process it...
                    // ZZZ for now, just print it
                    mkspkt_id_t id_pkt;
                    _unpack_id_packet(&id_pkt, p);
                    debug_printf("MKS ID: Bytes:%hi SeqNo:%i idflag:%i ID:'%s' Version:'%s'\n", 
                        id_pkt.bytes, id_pkt.seqno, id_pkt.idflag, id_pkt.id, id_pkt.version);
                }
            }
            pbuf_free(p);
        }
    }
    else {
        error_printf("MKOB Wire receive called without a message. Host:Port %d:%d\n", ip_addr->addr, port);
    }
}

// Continuation of the `_send_id` function. Called after 'ACK' is received.
static void _send_id_2() {
    if (_udp_pcb) {
        _seqno_send += 2; // Account for the CON message and this DAT message.
        struct pbuf* p = _send_id_req_builder();
        err_enum_t status = udp_send(_udp_pcb, p);
        debug_printf("MKS send_id send status: %d\n", status);
        pbuf_free(p);
        _start_keep_alive();
    }
}

static void _send_id() {
    if (_udp_pcb) {
        // Send a connect message and indicate that _send_id_2 should be called when 'ACK' is received.
        struct pbuf* p = _connect_req_builder();
        err_enum_t status = udp_send(_udp_pcb, p);
        debug_printf("MKS connect send status: %d\n", status);
        pbuf_free(p);
        _next_fn = _send_id_2;
    }
}

bool _keep_alive_callback(struct repeating_timer* t) {
    // debug_printf("Keep alive at %lld\n", time_us_64());
    _send_id();
    return true;
}

static void _start_keep_alive() {
    if (!_keep_alive_active) {
        add_repeating_timer_ms(MKS_KEEP_ALIVE_TIME, _keep_alive_callback, NULL, &_keep_alive_timer);
        _keep_alive_active = true;
    }
}

static void _stop_keep_alive() {
    if (_keep_alive_active) {
        _keep_alive_active = !cancel_repeating_timer(&_keep_alive_timer);
    }
}

static void _wire_connect() {
    if (_udp_pcb != NULL) {
        mkwire_disconnect();
    }
    // Need to bind to a port to keep it constant across UDP operations,
    // as KOBServer tracks clients by IP:Port
    err_enum_t status = udp_socket_bind(_mkserver_host, _mkserver_port, _bind_handler);
    if (!(ERR_OK == status || ERR_INPROGRESS == status)) {
        error_printf("MK Wire Connect failed: %d\n", status);
    }
}
