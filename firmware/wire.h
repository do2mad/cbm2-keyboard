// CBM2 Keyboard – cable link to the Pico64 Keyboard (instead of Bluetooth)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_WIRE_H
#define CBM2_WIRE_H

#include <stdbool.h>
#include "kb.h"

void wire_init(void);
/** Call every 1 ms: checks the cable, sends the state when changed and every 100 ms. */
void wire_tick(const kb_state_t *s);
bool wire_connected(void);           // cable plugged in (detect line low)

#endif
