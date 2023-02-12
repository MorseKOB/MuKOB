/**
 * KOB Network functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#include "net.h"
#include "hardware/rtc.h"

static char _wifi_ssid[NET_SSID_MAX_LEN];
static char _wifi_password[NET_PASSWORD_MAX_LEN];

static bool _wifi_connected = false;

// Forward definitions...
static void _ntp_response_handler(err_enum_t status, struct pbuf* p);
static void _udp_bind_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg);
static int64_t _udp_bind_dns_timeout_handler(alarm_id_t id, void* request_state);
static void _udp_sop_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg);
static void _udp_sop_recv(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* addr, u16_t port);
static int64_t _udp_sop_timeout_handler(alarm_id_t id, void* request_state);

struct _udp_op_context {
    err_enum_t status;
    ip_addr_t ipaddr;
    uint16_t port;
    struct udp_pcb* udp_pcb;
    uint32_t timeout_ms;
    alarm_id_t timeout_alarm_id;
    struct pbuf* p;
    udp_sop_result_handler_fn op_result_handler;
    udp_bind_handler_fn bind_handler;
};
#define ANY_LOCAL_PORT 0 // Used in udp_bind

#define DNS_TIMEOUT (5 * 1000)
#define UDP_SO_FAILSAFE_TO (60 * 1000)

#define NTP_SERVER "pool.ntp.org"
#define NTP_PORT 123
#define NTP_TIMEOUT (10 * 1000)
#define NTP_MSG_LEN 48
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970


// Public functions...

bool wifi_connect() {
    if (!_wifi_connected) {
        if (cyw43_arch_wifi_connect_timeout_ms(_wifi_ssid, _wifi_password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
            error_printf("failed to connect\n");
            return (false);
        }
        _wifi_connected = true;
    }
    return (true);
}

bool wifi_connected() {
    return (_wifi_connected);
}

void wifi_set_creds(const char* ssid, const char* pw) {
    strncpy(_wifi_ssid, ssid, NET_SSID_MAX_LEN);
    strncpy(_wifi_password, pw, NET_PASSWORD_MAX_LEN);
}

err_enum_t network_update_rtc() {
    // Build the NTP request message...
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t* req = (uint8_t*)p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b; // NTP Request: Version=3 Mode=3 (client)

    err_enum_t status = udp_single_operation(NTP_SERVER, NTP_PORT, p, NTP_TIMEOUT, _ntp_response_handler);
    if (status != ERR_OK && status != ERR_INPROGRESS) {
        // Operation initialization failed. Need to free the PBUF we created...
        pbuf_free(p);
    }
    return (status);
}

err_enum_t udp_socket_bind(const char* hostname, uint16_t port, udp_bind_handler_fn bind_handler) {
    err_enum_t status = ERR_INPROGRESS;

    if (!wifi_connect()) {
        return ERR_CONN;
    }
    struct _udp_op_context* op_context = malloc(sizeof(struct _udp_op_context));
    if (!op_context) {
        error_printf("UDP Single Operation - failed to allocate context\n");
        return ERR_MEM;
    }

    op_context->port = port;
    op_context->bind_handler = bind_handler;

    // Set up a timeout so we can call the bind handler even if the DNS lookup fails.
    op_context->timeout_alarm_id = add_alarm_in_ms(DNS_TIMEOUT, _udp_bind_dns_timeout_handler, op_context, true);
    debug_printf("Set udp_socket_bind DNS timeout: %d  (%ums)\n", op_context->timeout_alarm_id, DNS_TIMEOUT);
    cyw43_arch_lwip_begin();
    {
        status = dns_gethostbyname_addrtype(hostname, &op_context->ipaddr, _udp_bind_dns_found, op_context, LWIP_DNS_ADDRTYPE_IPV4_IPV6);
    }
    cyw43_arch_lwip_end();

    if (status == ERR_OK) {
        // The address is ready. Continue with our processing...
        op_context->status = status;
        _udp_bind_dns_found(hostname, &op_context->ipaddr, (void*)op_context);
    }
    else if (status != ERR_INPROGRESS) { // ERR_INPROGRESS means expect a callback
        error_printf("DNS request failed\n");
        op_context->status = status;
    }

    return (status);
}

err_enum_t udp_single_operation(const char* hostname, uint16_t port, struct pbuf* p, uint32_t timeout_ms, udp_sop_result_handler_fn result_handler) {
    err_enum_t status = ERR_INPROGRESS;

    if (!wifi_connect()) {
        return ERR_CONN;
    }
    struct _udp_op_context* op_context = malloc(sizeof(struct _udp_op_context));
    if (!op_context) {
        error_printf("UDP Single Operation - failed to allocate context\n");
        return ERR_MEM;
    }

    op_context->port = port;
    op_context->timeout_ms = timeout_ms;
    op_context->p = p;
    op_context->op_result_handler = result_handler;

    cyw43_arch_lwip_begin();
    {
        status = dns_gethostbyname_addrtype(hostname, &op_context->ipaddr, _udp_sop_dns_found, op_context, LWIP_DNS_ADDRTYPE_IPV4_IPV6);
    }
    cyw43_arch_lwip_end();

    if (status == ERR_OK) {
        // The address is ready (maybe it was in octet format). Call the bind handler...
        op_context->status = status;
        _udp_sop_dns_found(hostname, &op_context->ipaddr, op_context);
    }
    else if (status != ERR_INPROGRESS) { // ERR_INPROGRESS means expect a callback
        error_printf("UDP Single Operation DNS request failed\n");
        free(op_context);
    }

    return (status);
}

// Called with pre-processed results of NTP operation
static void _ntp_set_datetime(err_enum_t status, time_t* result) {
    if (status == ERR_OK && result) {
        struct tm* utc = gmtime(result);
        debug_printf("NTP response: %02d/%02d/%04d %02d:%02d:%02d\n", utc->tm_mon + 1,
            utc->tm_mday, utc->tm_year + 1900, utc->tm_hour, utc->tm_min,
            utc->tm_sec);
        // Set the board's time
        datetime_t tm;
        tm.day = utc->tm_mday;
        tm.month = utc->tm_mon + 1;
        tm.year = utc->tm_year + 1900;
        tm.dotw = utc->tm_wday;
        tm.hour = utc->tm_hour;
        tm.min = utc->tm_min;
        tm.sec = utc->tm_sec;
        rtc_set_datetime(&tm);
    }
}

// NTP data received (udp_incoming_data_handler_fn)
static void _ntp_response_handler(err_enum_t status, struct pbuf* p) {
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    if (status != ERR_OK) {
        _ntp_set_datetime(status, NULL);
    }
    else if (status == ERR_OK && mode == 0x4 && stratum != 0) {
        uint8_t seconds_buf[4] = { 0 };
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
        time_t epoch = seconds_since_1970;
        _ntp_set_datetime(status, &epoch);
    }
    else {
        error_printf("invalid NTP response\n");
        _ntp_set_datetime(ERR_VAL, NULL);
    }
    pbuf_free(p);
}

// Called back with a DNS result (dns_found_callback)
static void _udp_bind_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg) {
    struct _udp_op_context* op_context = (struct _udp_op_context*)arg;
    udp_bind_handler_fn bind_handler = op_context->bind_handler;
    err_enum_t status = ERR_OK;
    struct udp_pcb* udp_pcb = NULL;

    // Cancel the pending timeout for the DNS operation.
    if (op_context->timeout_alarm_id != 0) {
        debug_printf("Cancel udp_socket_bind DNS timeout: %d\n", op_context->timeout_alarm_id);
        cancel_alarm(op_context->timeout_alarm_id);
        op_context->timeout_alarm_id = 0;
    }

    if (ipaddr) {
        // Bind a UDP PCB to this remote address and hand it off
        udp_pcb = udp_new();
        if (udp_pcb) {
            status = udp_bind(udp_pcb, IP_ANY_TYPE, ANY_LOCAL_PORT);
            if (status == ERR_OK) {
                status = udp_connect(udp_pcb, ipaddr, op_context->port);
                if (status != ERR_OK) {
                    udp_remove(udp_pcb);
                    udp_pcb = NULL;
                    error_printf("UDP Bind could not connect to %s:%hu\n", hostname, op_context->port);
                }
            }
            else {
                udp_remove(udp_pcb);
                udp_pcb = NULL;
                error_printf("UDP Bind could not bind to local interface\n");
            }
        }
        else {
            error_printf("UDP Bind could not allocate a UDP PCB\n");
        }
    }
    else {
        error_printf("UDP Bind DNS request failed for hostname: '%s'\n", hostname);
        status = ERR_RTE;
    }

    // All done. Free resources and call their bind handler.
    free(op_context);
    bind_handler(status, udp_pcb);
}

// Called on timeout of DNS lookup (alarm_callback_t)
static int64_t _udp_bind_dns_timeout_handler(alarm_id_t id, void* request_state) {
    struct _udp_op_context* op_context = (struct _udp_op_context*)request_state;
    udp_bind_handler_fn bind_handler = op_context->bind_handler;

    cancel_alarm(id);
    error_printf("UDP Bind DNS request failed with timeout (id:%d timeout_id:%d)\n", id, op_context->timeout_alarm_id);

    free(op_context);

    bind_handler(ERR_TIMEOUT, NULL);

    return 0; // Don't reschedule this alarm.
}

// Called back by the DNS lookup from a UDP_SINGLE_OPERATION call.
static void _udp_sop_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg) {
    struct _udp_op_context* op_context = (struct _udp_op_context*)arg;

    struct pbuf* p = op_context->p;
    udp_sop_result_handler_fn op_result_handler = op_context->op_result_handler;

    err_enum_t status = ERR_ABRT;
    if (ipaddr) {
        op_context->ipaddr = *ipaddr;
        // set up for receiving the result message and send the outgoing message
        op_context->udp_pcb = udp_new();
        if (op_context->udp_pcb) {
            // set up to receive a response and create a timeout.
            udp_recv(op_context->udp_pcb, _udp_sop_recv, op_context);
            status = udp_bind(op_context->udp_pcb, IP_ANY_TYPE, 0);
            if (status == ERR_OK) {
                status = udp_sendto(op_context->udp_pcb, op_context->p, ipaddr, op_context->port);
                if (status == ERR_OK) {
                    // Free this outgoing message PBUF
                    pbuf_free(p);
                    // Set up a timeout so we can free things up and call the handler even we don't receive a response.
                    uint32_t toms = (op_context->timeout_ms > 0 ? op_context->timeout_ms : UDP_SO_FAILSAFE_TO);
                    op_context->timeout_alarm_id = add_alarm_in_ms(toms, _udp_sop_timeout_handler, op_context, true);
                    debug_printf("Set udp_single_operation timeout: %d  (%ums)\n", op_context->timeout_alarm_id, toms);

                    return;
                }
                // status set
                error_printf("UDP Op - Error sending message: %d\n", status);
            }
            else {
                // status set
                error_printf("UDP Op - Cannot bind\n");
            }
        }
        else {
            status = ERR_MEM;
            error_printf("UDP Op - Cannot create PCB\n");
        }
    }
    else {
        status = ERR_RTE;
        error_printf("UDP Op - DNS request failed for hostname: '%s'\n", hostname);
    }
    // If we get here it means that there was a problem. Free resources.
    free(op_context);
    free(op_context);
    // Call thier handler and give them thier PBUF back. They are set up to free one anyway.
    op_result_handler(status, p);
}

// UDP operation data received for a single operation (udp_recv_fn)
static void _udp_sop_recv(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* addr, u16_t port) {
    struct _udp_op_context* op_context = (struct _udp_op_context*)arg;

    udp_sop_result_handler_fn op_result_handler = op_context->op_result_handler;

    // Cancel the pending timeout for this operation.
    if (op_context->timeout_alarm_id != 0) {
        debug_printf("Cancel udp_single_operation timeout: %d\n", op_context->timeout_alarm_id);
        cancel_alarm(op_context->timeout_alarm_id);
        op_context->timeout_alarm_id = 0;
    }

    // Hold on to some stuff and free our contexts
    ip_addr_t raddr = op_context->ipaddr;
    uint16_t rport = op_context->port;

    free(op_context);
    udp_remove(pcb);

    // Do a sanity check on the response.
    if (ip_addr_cmp(addr, &raddr) && port == rport) {
        op_result_handler(ERR_OK, p);
    }
    else {
        op_result_handler(ERR_RTE, p);
    }
}

// Called on timeout of a single operation (no message received) (alarm_callback_t)
static int64_t _udp_sop_timeout_handler(alarm_id_t id, void* request_state) {
    struct _udp_op_context* op_context = (struct _udp_op_context*)request_state;

    cancel_alarm(id);
    error_printf("UDP - Single operation, timeout waiting for response (id:%d timeout_id:%d)\n", id, op_context->timeout_alarm_id);

    struct pbuf* p = op_context->p;
    udp_sop_result_handler_fn op_result_handler = op_context->op_result_handler;

    // Free the resources
    udp_remove(op_context->udp_pcb);
    free(op_context);
    free(op_context);

    // Call thier handler and give them thier PBUF back. They are set up to free one anyway.
    op_result_handler(ERR_TIMEOUT, p);

    return 0; // Don't reschedule this alarm.
}
