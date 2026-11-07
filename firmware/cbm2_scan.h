// CBM2 Keyboard – scans the CBM-II keyboard matrix (16 columns x 6 rows)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_SCAN_H
#define CBM2_SCAN_H

#include <stdint.h>
#include <stdbool.h>

#define CBM_ROWS 6
#define CBM_COLS 16
#define CBM_KEYS (CBM_ROWS * CBM_COLS)
#define CBM_KEY(row, col) ((uint8_t)((row) * CBM_COLS + (col)))   // row C0..C5, column R0..R15

#define CBM_MAX_HELD 12

typedef struct {
    uint8_t held[CBM_MAX_HELD];   // keys held, in the order they were pressed
    int     n;
} cbm_keys_t;

void cbm_scan_init(void);
/** Scan once (call every 2 ms). Returns true if the debounced state changed. */
bool cbm_scan(void);
const cbm_keys_t *cbm_keys(void);
bool cbm_is_held(uint8_t key);
const char *cbm_key_name(uint8_t key);

#endif
