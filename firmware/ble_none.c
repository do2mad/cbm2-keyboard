// CBM2 Keyboard – no Bluetooth (RP2040 boards without radio): stubs for ble_link.h
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#include <stdio.h>
#include "ble_link.h"

void ble_link_init(void) {}
bool ble_link_ready(void) { return false; }
bool ble_link_scanning(void) { return false; }
void ble_link_send(const kb_state_t *s) { (void)s; }
void ble_link_tick(void) {}
void ble_link_status(void) { printf("Bluetooth: none (RP2040 board, cable only)\n"); }
void ble_link_reconnect(void) {}
void ble_link_enable(bool on) { (void)on; }
