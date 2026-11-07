// CBM2 Keyboard – Bluetooth LE link to a Pico64 Keyboard (or BT-64 / Blue-64)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_BLE_LINK_H
#define CBM2_BLE_LINK_H

#include <stdbool.h>
#include "kb.h"

void ble_link_init(void);            // before hci_power_control(HCI_POWER_ON)
bool ble_link_ready(void);           // connected and characteristic found
bool ble_link_scanning(void);
void ble_link_send(const kb_state_t *s);   // new state (sent as soon as possible)
void ble_link_tick(void);            // call every 2 ms: retries pending writes
void ble_link_status(void);
void ble_link_reconnect(void);       // disconnect and search again
void ble_link_enable(bool on);       // off: disconnect, stop searching (cable in use)

#endif
