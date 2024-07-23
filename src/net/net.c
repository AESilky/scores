/**
 * Network functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#include "net.h"

#include "board.h"

#include "util/util.h"

#include "hardware/rtc.h"
#include "pico/time.h"

#include <stdlib.h>
#include <time.h>

#define ADDR_PORT_SEP ':'
static bool _wifi_connected = false;


#ifdef BOARD_IS_PICOW  // Only provide WiFi features if this is a Pico-W
static char _wifi_ssid[NET_SSID_MAX_LEN];
static char _wifi_password[NET_PASSWORD_MAX_LEN];

// Forward definitions...
/**
 * @brief Function prototype for UDP Bind response handler.
 * @ingroup wire
 *
 * @param status The status from the operation.
 * @param udp_pcb The udp_pcb that was bound, or NULL if an error occurred.
 */
typedef void (*udp_bind_handler_fn)(err_enum_t status, struct udp_pcb* udp_pcb);

/**
 * @brief Function prototype for UDP single operation result handler.
 * @ingroup wire
 *
 * @param status The status from the operation.
 * @param p The PBUF from the UDP operation. It is the handler's responsibility to free the pbuf.
 */
typedef void (*udp_sop_result_handler_fn)(err_enum_t status, pbuf_t* p, void* handler_data);


/**
 * @brief Send a UDP message and process the response message.
 * @ingroup wire
 *
 * @param hostname The fully qualified name of the host. This will be used to do a DNS lookup to obtain an IP address.
 * @param port A port number to use when sending the request.
 * @param bind_handler A function to be called after the hostname is resolved and a UDP socket is bound.
 *
 * @returns Error number (from err.h). ERR_OK or ERR_INPROGRESS is returned on success.
 */
err_enum_t udp_socket_bind(const char* hostname, uint16_t port, udp_bind_handler_fn bind_handler);

/**
 * @brief Perform a single UDP operation, consisting of sending a message and getting a response message.
 * @ingroup wire
 *
 * @param hostname The fully qualified name of the host. This will be used to do a DNS lookup to obtain an IP address.
 * @param port A port number to use when sending the request.
 * @param p A PBUF structure containing the message to be sent.
 * @param timeout A millisecond timeout value to use for the request/response. If a timeout occurs, the result handler
 *          will be called with a status of ERR_TOUT and a NULL PBUF.
 * @param result_handler Function called with the result when it is received.
 * @param handler_data Data that is passed to the result handler along with the response.
 *
 * @returns Error number (from err.h). ERR_OK or ERR_INPROGRESS is returned on success.
 */
err_enum_t udp_single_operation(const char* hostname, uint16_t port, pbuf_t* p, uint32_t timeout, udp_sop_result_handler_fn result_handler, void* handler_data);

static void _ntp_response_handler(err_enum_t status, pbuf_t* p, void* handler_data);
static void _udp_bind_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg);
static int64_t _udp_bind_dns_timeout_handler(alarm_id_t id, void* request_state);
static void _udp_sop_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg);
static void _udp_sop_recv(void* arg, struct udp_pcb* pcb, pbuf_t* p, const ip_addr_t* addr, u16_t port);
static int64_t _udp_sop_timeout_handler(alarm_id_t id, void* request_state);

typedef struct _udp_op_context {
    err_enum_t status;
    ip_addr_t ipaddr;
    uint16_t port;
    struct udp_pcb* udp_pcb;
    uint32_t timeout_ms;
    alarm_id_t timeout_alarm_id;
    pbuf_t* p;
    udp_sop_result_handler_fn op_result_handler;
    void* result_handler_data;
    udp_bind_handler_fn bind_handler;
} udp_op_context_t;

#define ANY_LOCAL_PORT 0 // Used in udp_bind

#define DNS_TIMEOUT (5 * 1000)
#define UDP_SO_FAILSAFE_TO (60 * 1000)

#define NTP_SERVER "pool.ntp.org"
#define NTP_PORT 123
#define NTP_TIMEOUT (10 * 1000)
#define NTP_MSG_LEN 48
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
typedef struct _ntp_handler_data {
    float tz_offset;
} ntp_handler_data_t;


err_enum_t udp_socket_bind(const char* hostname, uint16_t port, udp_bind_handler_fn bind_handler) {
    err_enum_t status = ERR_INPROGRESS;

    if (!wifi_connect()) {
        return ERR_CONN;
    }
    udp_op_context_t* op_context = malloc(sizeof(udp_op_context_t));
    if (!op_context) {
        error_printf(false, "UDP Single Operation - failed to allocate context\n");
        return ERR_MEM;
    }

    op_context->port = port;
    op_context->bind_handler = bind_handler;

    // Set up a timeout so we can call the bind handler even if the DNS lookup fails.
    op_context->timeout_alarm_id = add_alarm_in_ms(DNS_TIMEOUT, _udp_bind_dns_timeout_handler, op_context, true);
    debug_printf(true, "Set udp_socket_bind DNS timeout: %d  (%ums)\n", op_context->timeout_alarm_id, DNS_TIMEOUT);
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
        error_printf(false, "DNS request failed\n");
        op_context->status = status;
    }

    return (status);
}

err_enum_t udp_single_operation(const char* hostname, uint16_t port, pbuf_t* p, uint32_t timeout_ms, udp_sop_result_handler_fn result_handler, void* handler_data) {
    err_enum_t status = ERR_INPROGRESS;

    if (!wifi_connect()) {
        return ERR_CONN;
    }
    udp_op_context_t* op_context = malloc(sizeof(udp_op_context_t));
    if (!op_context) {
        error_printf(false, "UDP Single Operation - failed to allocate context\n");
        return ERR_MEM;
    }

    op_context->port = port;
    op_context->timeout_ms = timeout_ms;
    op_context->p = p;
    op_context->op_result_handler = result_handler;
    op_context->result_handler_data = handler_data;

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
        error_printf(false, "UDP Single Operation DNS request failed\n");
        free(op_context);
    }

    return (status);
}

#endif // BOARD_IS_PICOW

// ====================================================================
// Public functions
// ====================================================================

int host_from_hostport(char* buf, uint32_t maxlen, const char* host_and_port) {
    int len_of_host;

    if (!host_and_port) {
        *buf = '\000';
        return (0);
    }
    // See if there is a ':'
    char* sep = strchr(host_and_port, ADDR_PORT_SEP);
    if (sep) {
        // There is a ':', get the string up to it
        len_of_host = sep - host_and_port;
    }
    else {
        len_of_host = strlen(host_and_port);
    }
    if (maxlen > len_of_host) {
        strcpynt(buf, host_and_port, len_of_host);
    }
    else {
        strcpynt(buf, host_and_port, maxlen);
    }

    return (len_of_host);
}

uint16_t port_from_hostport(const char* host_and_port, uint16_t port_default) {
    // Is there a ':'?
    char* sep = strchr(host_and_port, ADDR_PORT_SEP);
    if (NULL == sep) {
        return port_default;
    }
    return (atoi(sep + 1));
}

bool wifi_connect() {
#ifdef BOARD_IS_PICOW
    if (WiFiAvailable && !_wifi_connected) {
        if (cyw43_arch_wifi_connect_timeout_ms(_wifi_ssid, _wifi_password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
            error_printf(false, "failed to connect\n");
            return (false);
        }
        _wifi_connected = true;
    }
#endif // BOARD_IS_PICOW
    return (true);
}

bool wifi_connected() {
    return (_wifi_connected);
}

void wifi_set_creds(const char* ssid, const char* pw) {
#ifdef BOARD_IS_PICOW
    strcpynt(_wifi_ssid, ssid, NET_SSID_MAX_LEN);
    strcpynt(_wifi_password, pw, NET_PASSWORD_MAX_LEN);
#endif // BOARD_IS_PICOW
}

err_enum_t network_update_rtc(float tz_offset) {
#ifdef BOARD_IS_PICOW
    if (WiFiAvailable) {
        // Build the NTP request message...
        pbuf_t* p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_POOL);
        uint8_t* req = (uint8_t*)p->payload;
        memset(req, 0, NTP_MSG_LEN);
        req[0] = 0x1b; // NTP Request: Version=3 Mode=3 (client)
        ntp_handler_data_t* handler_data = (ntp_handler_data_t*)malloc(sizeof(ntp_handler_data_t));
        handler_data->tz_offset = tz_offset;

        err_enum_t status = udp_single_operation(NTP_SERVER, NTP_PORT, p, NTP_TIMEOUT, _ntp_response_handler, (void*)handler_data);
        if (status != ERR_OK && status != ERR_INPROGRESS) {
            // Operation initialization failed. Need to free the PBUF we created...
            pbuf_free(p);
        }
        return (status);
    }
#endif // BOARD_IS_PICOW
    return (ERR_OK);
}


// ====================================================================
// Internal functions
// ====================================================================

#ifdef BOARD_IS_PICOW

// Called with pre-processed results of NTP operation
static void _ntp_set_datetime(err_enum_t status, time_t* seconds_from_epoch, float tz_offset) {
    if (status == ERR_OK && seconds_from_epoch) {
        // Adjust by the tz_offset - hours from GMT
        time_t offset = (time_t)(3600.0 * tz_offset);
        *seconds_from_epoch += offset;
        // Get a UTC structure time
        struct tm* utc = gmtime(seconds_from_epoch);
        // Set the board's time
        datetime_t tm;
        tm.day = utc->tm_mday;
        tm.month = utc->tm_mon + 1;
        tm.year = utc->tm_year + 1900;
        tm.dotw = utc->tm_wday;
        tm.hour = utc->tm_hour;
        tm.min = utc->tm_min;
        tm.sec = utc->tm_sec;
        // Adjust for timezone

        rtc_set_datetime(&tm);
    }
}

// NTP data received (udp_incoming_data_handler_fn)
static void _ntp_response_handler(err_enum_t status, pbuf_t* p, void* handler_data) {
    if (p) {
        uint8_t mode = pbuf_get_at(p, 0) & 0x7;
        uint8_t stratum = pbuf_get_at(p, 1);
        ntp_handler_data_t* ntp_handler_data = (ntp_handler_data_t*)handler_data;
        float tz_offset = ntp_handler_data->tz_offset;

        if (status != ERR_OK) {
            _ntp_set_datetime(status, NULL, tz_offset);
        }
        else if (status == ERR_OK && mode == 0x4 && stratum != 0) {
            uint8_t seconds_buf[4] = { 0 };
            pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
            uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
            uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
            time_t seconds_from_epoch = seconds_since_1970;
            _ntp_set_datetime(status, &seconds_from_epoch, tz_offset);
        }
        else {
            error_printf(false, "invalid NTP response\n");
            _ntp_set_datetime(ERR_VAL, NULL, tz_offset);
        }
        if (p->ref > 0) {
            pbuf_free(p);
        }
    }
    if (handler_data) {
        free(handler_data);
    }
}

// Called back with a DNS result (dns_found_callback)
static void _udp_bind_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg) {
    udp_op_context_t* op_context = (udp_op_context_t*)arg;
    udp_bind_handler_fn bind_handler = op_context->bind_handler;
    err_enum_t status = ERR_OK;
    struct udp_pcb* udp_pcb = NULL;

    // Cancel the pending timeout for the DNS operation.
    if (op_context->timeout_alarm_id != 0) {
        debug_printf(true, "Cancel udp_socket_bind DNS timeout: %d\n", op_context->timeout_alarm_id);
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
                    error_printf(false, "UDP Bind could not connect to %s:%hu\n", hostname, op_context->port);
                }
            }
            else {
                udp_remove(udp_pcb);
                udp_pcb = NULL;
                error_printf(false, "UDP Bind could not bind to local interface\n");
            }
        }
        else {
            error_printf(false, "UDP Bind could not allocate a UDP PCB\n");
        }
    }
    else {
        error_printf(false, "UDP Bind DNS request failed for hostname: '%s'\n", hostname);
        status = ERR_RTE;
    }

    // All done. Free resources and call their bind handler.
    free(op_context);
    bind_handler(status, udp_pcb);
}

// Called on timeout of DNS lookup (alarm_callback_t)
static int64_t _udp_bind_dns_timeout_handler(alarm_id_t id, void* request_state) {
    udp_op_context_t* op_context = (udp_op_context_t*)request_state;
    udp_bind_handler_fn bind_handler = op_context->bind_handler;

    cancel_alarm(id);
    error_printf(false, "UDP Bind DNS request failed with timeout (id:%d timeout_id:%d)\n", id, op_context->timeout_alarm_id);

    free(op_context);

    bind_handler(ERR_TIMEOUT, NULL);

    return 0; // Don't reschedule this alarm.
}

// Called back by the DNS lookup from a UDP_SINGLE_OPERATION call.
static void _udp_sop_dns_found(const char* hostname, const ip_addr_t* ipaddr, void* arg) {
    udp_op_context_t* op_context = (udp_op_context_t*)arg;

    pbuf_t* p = op_context->p;
    udp_sop_result_handler_fn op_result_handler = op_context->op_result_handler;
    void* handler_data = op_context->result_handler_data;

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
                    debug_printf(true, "Set udp_single_operation timeout: %d  (%ums)\n", op_context->timeout_alarm_id, toms);

                    return;
                }
                // status set
                error_printf(false, "UDP Op - Error sending message: %d\n", status);
            }
            else {
                // status set
                error_printf(false, "UDP Op - Cannot bind\n");
            }
        }
        else {
            status = ERR_MEM;
            error_printf(false, "UDP Op - Cannot create PCB\n");
        }
    }
    else {
        status = ERR_RTE;
        error_printf(false, "UDP Op - DNS request failed for hostname: '%s'\n", hostname);
    }
    // If we get here it means that there was a problem. Free resources.
    free(op_context);
    // Call thier handler and give them thier PBUF back. They are set up to free one anyway.
    op_result_handler(status, p, handler_data);
}

// UDP operation data received for a single operation (udp_recv_fn)
static void _udp_sop_recv(void* arg, struct udp_pcb* pcb, pbuf_t* p, const ip_addr_t* addr, u16_t port) {
    udp_op_context_t* op_context = (udp_op_context_t*)arg;

    udp_sop_result_handler_fn op_result_handler = op_context->op_result_handler;

    // Cancel the pending timeout for this operation.
    if (op_context->timeout_alarm_id != 0) {
        debug_printf(true, "Cancel udp_single_operation timeout: %d\n", op_context->timeout_alarm_id);
        cancel_alarm(op_context->timeout_alarm_id);
        op_context->timeout_alarm_id = 0;
    }

    // Hold on to some stuff and free our contexts
    ip_addr_t raddr = op_context->ipaddr;
    uint16_t rport = op_context->port;
    void* handler_data = op_context->result_handler_data;

    free(op_context);
    udp_remove(pcb);

    // Do a sanity check on the response.
    if (ip_addr_cmp(addr, &raddr) && port == rport) {
        op_result_handler(ERR_OK, p, handler_data);
    }
    else {
        op_result_handler(ERR_RTE, p, handler_data);
    }
}

// Called on timeout of a single operation (no message received) (alarm_callback_t)
static int64_t _udp_sop_timeout_handler(alarm_id_t id, void* request_state) {
    udp_op_context_t* op_context = (udp_op_context_t*)request_state;

    cancel_alarm(id);
    error_printf(false, "UDP - Single operation, timeout waiting for response (id:%d timeout_id:%d)\n", id, op_context->timeout_alarm_id);

    pbuf_t* p = op_context->p;
    udp_sop_result_handler_fn op_result_handler = op_context->op_result_handler;

    void* handler_data = op_context->result_handler_data;
    // Free the resources
    udp_remove(op_context->udp_pcb);
    free(op_context);

    // Call thier handler and give them thier PBUF back. They are set up to free one anyway.
    op_result_handler(ERR_TIMEOUT, p, handler_data);

    return 0; // Don't reschedule this alarm.
}
#endif // BOARD_IS_PICOW
