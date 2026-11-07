// CBM2 Keyboard – Bluetooth LE link to a Pico64 Keyboard (or BT-64 / Blue-64)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// We are the central: scan for a device that advertises the BT-64 keyboard
// service C64B0001-..., connect, find the characteristics and send key states
// like the Blue-64 Keyboard app does:
//   Pico64 (info caps bit 0): full matrix C64B0005  [flags, col0..col7]
//   BT-64 / Blue-64:          one key      C64B0002  [flags, PA*8+PB]
// No pairing. After a disconnect the search starts again.

#include "ble_link.h"

#include <stdio.h>
#include <string.h>
#include "btstack.h"
#include "pico/time.h"

#define FLAG_SHIFT   0x01
#define FLAG_CMDR    0x02
#define FLAG_CTRL    0x04
#define FLAG_RESTORE 0x08

// big endian (as written)
static const uint8_t uuid_service[16] = { 0xC6,0x4B,0x00,0x01, 0xB1,0xE6, 0x4A,0x64, 0x9C,0x64, 0x6B,0x7E,0x3F,0x1A,0x2D,0x00 };
static const uint8_t uuid_key[16]     = { 0xC6,0x4B,0x00,0x02, 0xB1,0xE6, 0x4A,0x64, 0x9C,0x64, 0x6B,0x7E,0x3F,0x1A,0x2D,0x00 };
static const uint8_t uuid_info[16]    = { 0xC6,0x4B,0x00,0x04, 0xB1,0xE6, 0x4A,0x64, 0x9C,0x64, 0x6B,0x7E,0x3F,0x1A,0x2D,0x00 };
static const uint8_t uuid_matrix[16]  = { 0xC6,0x4B,0x00,0x05, 0xB1,0xE6, 0x4A,0x64, 0x9C,0x64, 0x6B,0x7E,0x3F,0x1A,0x2D,0x00 };

typedef enum { L_OFF, L_SCAN, L_CONNECT, L_SERVICE, L_CHARS, L_INFO, L_READY } link_state_t;

static link_state_t state = L_OFF;
static bool enabled = true;        // false while the cable to the Pico64 is plugged in
static bool hci_up;
static btstack_packet_callback_registration_t hci_cb;
static hci_con_handle_t con = HCI_CON_HANDLE_INVALID;
static bd_addr_t peer;
static bd_addr_type_t peer_type;
static char peer_name[24];
static gatt_client_service_t service;
static bool have_service;
static uint16_t h_key, h_info, h_matrix;
static uint8_t caps;

static kb_state_t want;
static bool dirty;
static absolute_time_t connect_deadline;

static absolute_time_t scan_slow_at;
static bool scan_slow;

static void start_scan(void) {
    con = HCI_CON_HANDLE_INVALID;
    if (!enabled || !hci_up) { state = L_OFF; return; }
    state = L_SCAN;
    scan_slow = false;
    scan_slow_at = make_timeout_time_ms(60000);
    con = HCI_CON_HANDLE_INVALID;
    have_service = false;
    h_key = h_info = h_matrix = 0;
    caps = 0;
    gap_set_scan_parameters(1, 0x30, 0x30);     // active scan (for the name), 30 ms of 30 ms
    gap_start_scan();
    printf("searching for Pico64 / BT-64 ...\n");
}

static void get_name(const uint8_t *adv, uint8_t len) {
    peer_name[0] = 0;
    ad_context_t ctx;
    for (ad_iterator_init(&ctx, len, adv); ad_iterator_has_more(&ctx); ad_iterator_next(&ctx)) {
        uint8_t t = ad_iterator_get_data_type(&ctx);
        if (t != BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME && t != BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME) continue;
        uint8_t n = ad_iterator_get_data_len(&ctx);
        if (n >= sizeof(peer_name)) n = sizeof(peer_name) - 1;
        memcpy(peer_name, ad_iterator_get_data(&ctx), n);
        peer_name[n] = 0;
    }
}

static void gatt_handler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)type; (void)channel; (void)size;
    uint8_t ev = hci_event_packet_get_type(packet);
    switch (state) {
    case L_SERVICE:
        if (ev == GATT_EVENT_SERVICE_QUERY_RESULT) {
            gatt_event_service_query_result_get_service(packet, &service);
            have_service = true;
        } else if (ev == GATT_EVENT_QUERY_COMPLETE) {
            if (!have_service) { printf("keyboard service not found\n"); gap_disconnect(con); break; }
            state = L_CHARS;
            gatt_client_discover_characteristics_for_service(gatt_handler, con, &service);
        }
        break;
    case L_CHARS:
        if (ev == GATT_EVENT_CHARACTERISTIC_QUERY_RESULT) {
            gatt_client_characteristic_t c;
            gatt_event_characteristic_query_result_get_characteristic(packet, &c);
            if (memcmp(c.uuid128, uuid_key, 16) == 0)    h_key = c.value_handle;
            if (memcmp(c.uuid128, uuid_info, 16) == 0)   h_info = c.value_handle;
            if (memcmp(c.uuid128, uuid_matrix, 16) == 0) h_matrix = c.value_handle;
        } else if (ev == GATT_EVENT_QUERY_COMPLETE) {
            if (!h_key && !h_matrix) { printf("key characteristic not found\n"); gap_disconnect(con); break; }
            if (h_info) {
                state = L_INFO;
                gatt_client_read_value_of_characteristic_using_value_handle(gatt_handler, con, h_info);
            } else {
                state = L_READY;
            }
        }
        break;
    case L_INFO:
        if (ev == GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT) {
            uint16_t n = gatt_event_characteristic_value_query_result_get_value_length(packet);
            const uint8_t *v = gatt_event_characteristic_value_query_result_get_value(packet);
            if (n >= 3) caps = v[2];
        } else if (ev == GATT_EVENT_QUERY_COMPLETE) {
            state = L_READY;
        }
        if (state == L_READY) {
            bool matrix = h_matrix && (caps & 1);
            printf("connected to %s (%s) - %s\n", peer_name[0] ? peer_name : "?", bd_addr_to_str(peer),
                   matrix ? "full matrix" : "single key (BT-64 protocol)");
            dirty = true;                       // send the current state
        }
        break;
    default:
        break;
    }
}

static void hci_handler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel; (void)size;
    if (type != HCI_EVENT_PACKET) return;
    switch (hci_event_packet_get_type(packet)) {
    case BTSTACK_EVENT_STATE:
        hci_up = btstack_event_state_get_state(packet) == HCI_STATE_WORKING;
        if (hci_up) start_scan();
        else state = L_OFF;
        break;
    case GAP_EVENT_ADVERTISING_REPORT: {
        if (state != L_SCAN) break;
        const uint8_t *adv = gap_event_advertising_report_get_data(packet);
        uint8_t len = gap_event_advertising_report_get_data_length(packet);
        if (!ad_data_contains_uuid128(len, adv, uuid_service)) break;
        gap_event_advertising_report_get_address(packet, peer);
        peer_type = (bd_addr_type_t)gap_event_advertising_report_get_address_type(packet);
        get_name(adv, len);
        state = L_CONNECT;
        connect_deadline = make_timeout_time_ms(5000);
        gap_stop_scan();
        printf("found %s (%s), connecting\n", peer_name[0] ? peer_name : "?", bd_addr_to_str(peer));
        gap_connect(peer, peer_type);
        break;
    }
    case HCI_EVENT_META_GAP:
        if (hci_event_gap_meta_get_subevent_code(packet) != GAP_SUBEVENT_LE_CONNECTION_COMPLETE) break;
        if (state != L_CONNECT) break;
        if (gap_subevent_le_connection_complete_get_status(packet) != ERROR_CODE_SUCCESS) {
            printf("connection failed\n");
            start_scan();
            break;
        }
        con = gap_subevent_le_connection_complete_get_connection_handle(packet);
        state = L_SERVICE;
        gatt_client_discover_primary_services_by_uuid128(gatt_handler, con, uuid_service);
        break;
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        if (hci_event_disconnection_complete_get_connection_handle(packet) != con) break;
        printf("Bluetooth disconnected\n");
        start_scan();
        break;
    default:
        break;
    }
}

void ble_link_init(void) {
    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    att_server_init(NULL, NULL, NULL);
    gatt_client_init();
    // connection interval 15-30 ms (units of 1.25 ms), like the app asks for
    gap_set_connection_parameters(0x30, 0x30, 12, 24, 0, 400, 0, 0);
    hci_cb.callback = &hci_handler;
    hci_add_event_handler(&hci_cb);
}

bool ble_link_ready(void) { return state == L_READY; }
bool ble_link_scanning(void) { return state == L_SCAN; }

void ble_link_send(const kb_state_t *s) {
    if (memcmp(s, &want, sizeof(want)) == 0) return;
    want = *s;
    dirty = true;
}

static bool write_matrix(void) {
    uint8_t buf[9];
    buf[0] = want.restore ? FLAG_RESTORE : 0;
    memcpy(&buf[1], want.cols, 8);
    return gatt_client_write_value_of_characteristic_without_response(con, h_matrix, sizeof(buf), buf) == ERROR_CODE_SUCCESS;
}

static bool write_key(void) {
    // BT-64 protocol: modifiers as flags + one key (the last one in matrix order)
    kb_state_t s = want;
    uint8_t flags = s.restore ? FLAG_RESTORE : 0;
    if (kb_held(&s, C_LSHIFT) || kb_held(&s, C_RSHIFT)) flags |= FLAG_SHIFT;
    if (kb_held(&s, C_CMDR)) flags |= FLAG_CMDR;
    if (kb_held(&s, C_CTRL)) flags |= FLAG_CTRL;
    uint8_t key = 0xFF;
    for (uint8_t c = 0; c < 64; c++) {
        if (c == C_LSHIFT || c == C_RSHIFT || c == C_CMDR || c == C_CTRL) continue;
        if (kb_held(&s, c)) key = c;
    }
    uint8_t buf[2] = { flags, key };
    return gatt_client_write_value_of_characteristic_without_response(con, h_key, sizeof(buf), buf) == ERROR_CODE_SUCCESS;
}

void ble_link_tick(void) {
    if (state == L_SCAN && !scan_slow && time_reached(scan_slow_at)) {
        // nothing found for a minute: keep searching, but only 10 % of the time (battery)
        scan_slow = true;
        gap_stop_scan();
        gap_set_scan_parameters(1, 0x1E0, 0x30);    // 30 ms every 300 ms
        gap_start_scan();
        printf("still searching (power saving)\n");
    }
    if (state == L_CONNECT && time_reached(connect_deadline)) {   // device gone meanwhile
        printf("no answer - searching again\n");
        gap_connect_cancel();
        start_scan();
        return;
    }
    if (state != L_READY || !dirty) return;
    bool ok = (h_matrix && (caps & 1)) ? write_matrix() : write_key();
    if (ok) dirty = false;                       // otherwise: buffer full, try again next tick
}

void ble_link_status(void) {
    static const char *const names[] = { "off", "searching", "connecting", "services", "characteristics", "info", "connected" };
    printf("Bluetooth: %s", (!enabled && state == L_OFF) ? "off (cable to the Pico64)" : names[state]);
    if (state >= L_CONNECT) printf(" - %s (%s)", peer_name[0] ? peer_name : "?", bd_addr_to_str(peer));
    if (state == L_READY) printf(", %s", (h_matrix && (caps & 1)) ? "full matrix" : "single key");
    printf("\n");
}

void ble_link_enable(bool on) {
    if (on == enabled) return;
    enabled = on;
    if (on) {
        printf("Bluetooth on\n");
        if (state == L_OFF) start_scan();
        return;
    }
    printf("Bluetooth off (cable)\n");
    if (state == L_SCAN)         { gap_stop_scan(); state = L_OFF; }
    else if (state == L_CONNECT) { gap_connect_cancel(); state = L_OFF; }
    else if (con != HCI_CON_HANDLE_INVALID) gap_disconnect(con);   // -> L_OFF when done
}

void ble_link_reconnect(void) {
    if (!enabled || state == L_SCAN) return;
    if (con != HCI_CON_HANDLE_INVALID) gap_disconnect(con);   // DISCONNECTION_COMPLETE starts the scan
    else if (state == L_CONNECT) { gap_connect_cancel(); start_scan(); }
    else if (state != L_OFF) start_scan();
}
