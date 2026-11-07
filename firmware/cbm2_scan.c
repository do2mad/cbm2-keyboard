// CBM2 Keyboard – scans the CBM-II keyboard matrix (16 columns x 6 rows)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// Wiring (measured on the 610 keyboard, 21.10.2026), the 22 used contacts of the
// keyboard plug in order, key position skipped:
//   GP0..GP5   = rows    C0..C5   (inputs with pull-up)
//   GP6..GP13  = columns R7..R0
//   GP14..GP21 = columns R15..R8
// One column at a time is pulled low, all others stay inputs (the keyboard has no
// diodes, so two driven columns could be shorted through two keys).

#include "cbm2_scan.h"

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define ROW_MASK   0x3Fu
#define LINES      22
#define DEBOUNCE   3          // scans with the same result (2 ms apart)
#define SETTLE_US  8

static const char *const names[CBM_ROWS][CBM_COLS] = {
 {"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","CRSR DOWN","CRSR UP","CLR/HOME","OFF/RVS","NORM/GRAPH","RUN/STOP"},
 {"ESC","1","2","3","4","5","7","8","9","0","=","CRSR LEFT","KP ?","KP CE","KP *","KP /"},
 {"TAB","Q","W","E","R","6","U","I","O","-","<-/POUND","CRSR RIGHT","KP 7","KP 8","KP 9","KP -"},
 {"(n/c)","A","S","D","T","Y","J","K","L","P","]","INS/DEL","KP 4","KP 5","KP 6","KP +"},
 {"SHIFT","Z","X","F","G","H","M",",",";","[","RETURN","C=","KP 1","KP 2","KP 3","ENTER"},
 {"CTRL","(n/c)","C","V","B","N","SPACE",".","/","'","PI","(n/c)","KP 0","KP .","KP 00","(n/c)"},
};

static uint8_t col_gpio[CBM_COLS];
static uint8_t raw[CBM_COLS], last_raw[CBM_COLS], stable[CBM_COLS];
static int same_count;
static cbm_keys_t keys;

void cbm_scan_init(void) {
    for (int c = 0; c < CBM_COLS; c++)
        col_gpio[c] = (uint8_t)(c < 8 ? 13 - c : 29 - c);     // R0..R7 = GP13..GP6, R8..R15 = GP21..GP14
    for (int i = 0; i < LINES; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_put(i, 0);
    }
}

static void update_order(void) {
    // remove released keys, keep the order of the rest
    int w = 0;
    for (int i = 0; i < keys.n; i++) {
        uint8_t k = keys.held[i];
        if (stable[k % CBM_COLS] & (1u << (k / CBM_COLS))) keys.held[w++] = k;
    }
    keys.n = w;
    // append newly pressed keys
    for (int c = 0; c < CBM_COLS; c++) {
        for (int r = 0; r < CBM_ROWS; r++) {
            if (!(stable[c] & (1u << r))) continue;
            uint8_t k = CBM_KEY(r, c);
            bool known = false;
            for (int i = 0; i < keys.n; i++) if (keys.held[i] == k) { known = true; break; }
            if (!known && keys.n < CBM_MAX_HELD) keys.held[keys.n++] = k;
        }
    }
}

bool cbm_scan(void) {
    for (int c = 0; c < CBM_COLS; c++) {
        uint g = col_gpio[c];
        gpio_set_dir(g, GPIO_OUT);              // output low
        busy_wait_us(SETTLE_US);
        raw[c] = (uint8_t)(~gpio_get_all() & ROW_MASK);
        gpio_set_dir(g, GPIO_IN);
    }
    if (memcmp(raw, last_raw, sizeof(raw)) != 0) {
        memcpy(last_raw, raw, sizeof(raw));
        same_count = 0;
        return false;
    }
    if (same_count < DEBOUNCE) same_count++;
    if (same_count != DEBOUNCE - 1) return false;
    if (memcmp(raw, stable, sizeof(raw)) == 0) return false;
    memcpy(stable, raw, sizeof(raw));
    update_order();
    return true;
}

const cbm_keys_t *cbm_keys(void) { return &keys; }

bool cbm_is_held(uint8_t key) {
    return key < CBM_KEYS && (stable[key % CBM_COLS] & (1u << (key / CBM_COLS)));
}

const char *cbm_key_name(uint8_t key) {
    return key < CBM_KEYS ? names[key / CBM_COLS][key % CBM_COLS] : "?";
}
