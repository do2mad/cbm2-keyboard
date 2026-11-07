// CBM2 Keyboard – CBM-II keys -> C64 keyboard
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_C64_MAP_H
#define CBM2_C64_MAP_H

#include "kb.h"
#include "cbm2_scan.h"

/** C64 keyboard state for the CBM-II keys held now. */
void c64_map(const cbm_keys_t *k, kb_state_t *out);

/** Text for a macro key (F9, F10, keypad 00), NULL for other keys. */
const char *c64_macro(uint8_t cbm_key);

#endif
