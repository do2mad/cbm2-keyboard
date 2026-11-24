// C64 Keyboard (on the CBM2 Keyboard PCB) – scans an original C64 keyboard (8 x 8 + RESTORE)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef C64KB_SCAN_H
#define C64KB_SCAN_H

#include <stdbool.h>
#include "kb.h"

void c64_scan_init(void);
/** Scan once. Returns true if the debounced state changed. */
bool c64_scan(void);
/** Debounced key state: cols[n] = PB rows pressed in PA column n, plus RESTORE. */
const kb_state_t *c64_keys(void);
/** Number of keys held (RESTORE counts too). */
int c64_held_count(void);
const char *c64_key_name(uint8_t code);

#endif
