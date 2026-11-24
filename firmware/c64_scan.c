// C64 Keyboard (on the CBM2 Keyboard PCB) – scans an original C64 keyboard (8 x 8 + RESTORE)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// The 20-pin plug of the C64 keyboard sits TURNED ROUND at the end of the 23-pin
// header J1 of the CBM2 Keyboard PCB: C64 pin 1 on J1 pin 23, the key position
// (C64 pin 2) on the key position J1 pin 22, C64 pin 20 on J1 pin 4. J1 pins 1-3
// stay free. J1 pin n = GP(n-1), J1 pin 23 = GP21, so C64 pin k = GP(23-k):
//
//   C64 pin  1  GND      GP21  driven low (ground for RESTORE)
//   C64 pin  3  RESTORE  GP20  input, pull-up
//   C64 pin  4  +5 V     GP19  left as input (not used; some keyboards with LEDs
//                              need real 5 V here - a GPIO cannot supply that)
//   C64 pin 13,19,18,17,16,15,14,20 = PA0..PA7 (columns, driven low one at a time)
//   C64 pin 12,11,10, 5, 8, 7, 6, 9 = PB0..PB7 (rows, inputs with pull-up)
//
// Like in the C64: one column low, the others inputs, rows read. The keyboard has no
// diodes, so some three-key combinations show a ghost key - just like on the C64.
// SHIFT LOCK is wired parallel to the left SHIFT inside the keyboard.

#include "c64_scan.h"

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define C64_PIN_GP(k)  ((uint)(23 - (k)))      // C64 plug pin k (3..20) -> GPIO
#define GP_GND      21
#define GP_RESTORE  20
#define GP_5V       19
#define DEBOUNCE    3                           // equal scans
#define SETTLE_US   8

static const uint8_t pa_pin[8] = { 13, 19, 18, 17, 16, 15, 14, 20 };   // C64 plug pins of PA0..PA7
static const uint8_t pb_pin[8] = { 12, 11, 10,  5,  8,  7,  6,  9 };   // C64 plug pins of PB0..PB7
static uint pa_gp[8], pb_gp[8];
static uint32_t pb_mask;

static kb_state_t raw, last_raw, stable;
static int same_count;

static const char *const names[64] = {
    "INST/DEL", "RETURN", "CRSR LR", "F7", "F1", "F3", "F5", "CRSR UD",
    "3", "W", "A", "4", "Z", "S", "E", "LSHIFT",
    "5", "R", "D", "6", "C", "F", "T", "X",
    "7", "Y", "G", "8", "B", "H", "U", "V",
    "9", "I", "J", "0", "M", "K", "O", "N",
    "+", "P", "L", "-", ".", ":", "@", ",",
    "POUND", "*", ";", "CLR/HOME", "RSHIFT", "=", "UP ARROW", "/",
    "1", "LEFT ARROW", "CTRL", "2", "SPACE", "C=", "Q", "RUN/STOP",
};

void c64_scan_init(void) {
    for (int i = 0; i < 8; i++) {
        pa_gp[i] = C64_PIN_GP(pa_pin[i]);
        pb_gp[i] = C64_PIN_GP(pb_pin[i]);
        pb_mask |= 1u << pb_gp[i];
    }
    for (uint g = 0; g <= 22; g++) {            // GP0..GP22: everything on J1 + cable detect
        if (g == 22) continue;                  // GP22 = cable detect (wire.c)
        gpio_init(g);
        gpio_set_dir(g, GPIO_IN);
        gpio_put(g, 0);
        gpio_disable_pulls(g);
    }
    for (int i = 0; i < 8; i++) { gpio_pull_up(pa_gp[i]); gpio_pull_up(pb_gp[i]); }
    gpio_pull_up(GP_RESTORE);
    gpio_set_dir(GP_GND, GPIO_OUT);             // keyboard ground, stays low
}

bool c64_scan(void) {
    memset(&raw, 0, sizeof(raw));
    for (int c = 0; c < 8; c++) {
        uint g = pa_gp[c];
        gpio_set_dir(g, GPIO_OUT);              // output low
        busy_wait_us(SETTLE_US);
        uint32_t in = ~gpio_get_all() & pb_mask;
        gpio_set_dir(g, GPIO_IN);
        for (int r = 0; r < 8; r++)
            if (in & (1u << pb_gp[r])) raw.cols[c] |= (uint8_t)(1u << r);
    }
    raw.restore = !gpio_get(GP_RESTORE);

    if (memcmp(&raw, &last_raw, sizeof(raw)) != 0) {
        last_raw = raw;
        same_count = 0;
        return false;
    }
    if (same_count < DEBOUNCE) same_count++;
    if (same_count != DEBOUNCE - 1) return false;
    if (memcmp(&raw, &stable, sizeof(raw)) == 0) return false;
    stable = raw;
    return true;
}

const kb_state_t *c64_keys(void) { return &stable; }

int c64_held_count(void) {
    int n = stable.restore ? 1 : 0;
    for (int c = 0; c < 8; c++) n += __builtin_popcount(stable.cols[c]);
    return n;
}

const char *c64_key_name(uint8_t code) { return code < 64 ? names[code] : "?"; }
