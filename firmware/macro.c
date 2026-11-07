// CBM2 Keyboard – types short texts (macros) on the C64 keyboard
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// Every character: key pressed for PRESS_MS, released for RELEASE_MS - long
// enough for the KERNAL (scans every 1/60 s), also over Bluetooth.

#include "macro.h"

#include <stdio.h>
#include <string.h>
#include "pico/time.h"

#define PRESS_MS   70
#define RELEASE_MS 50

static const uint8_t letters[26] = {
    C_A, C_B, C_C, C_D, C_E, C_F, C_G, C_H, C_I, C_J, C_K, C_L, C_M,
    C_N, C_O, C_P, C_Q, C_R, C_S, C_T, C_U, C_V, C_W, C_X, C_Y, C_Z,
};
static const uint8_t digits[10] = { C_0, C_1, C_2, C_3, C_4, C_5, C_6, C_7, C_8, C_9 };

static const struct { char c; uint8_t code; bool shift; } symbols[] = {
    {' ', C_SPACE, false}, {'\r', C_RETURN, false}, {'"', C_2, true}, {'*', C_STAR, false},
    {',', C_COMMA, false}, {'.', C_PERIOD, false}, {':', C_COLON, false}, {';', C_SEMI, false},
    {'/', C_SLASH, false}, {'=', C_EQUAL, false}, {'+', C_PLUS, false}, {'-', C_MINUS, false},
    {'@', C_AT, false}, {'!', C_1, true}, {'$', C_4, true}, {'%', C_5, true}, {'(', C_8, true}, {')', C_9, true},
};

static char text_buf[48];
static const char *text;
static size_t pos;
static bool pressed;
static absolute_time_t next;
static kb_state_t current;

bool macro_start(const char *t) {
    if (text || !t) return false;
    snprintf(text_buf, sizeof(text_buf), "%s", t);      // the caller's buffer may change
    text = text_buf;
    pos = 0;
    pressed = false;
    next = get_absolute_time();
    memset(&current, 0, sizeof(current));
    return true;
}

bool macro_busy(void) { return text != 0; }

static bool key_for(char c, kb_state_t *s) {
    memset(s, 0, sizeof(*s));
    uint8_t code = C_NONE;
    bool shift = false;
    if (c >= 'a' && c <= 'z') code = letters[c - 'a'];
    else if (c >= 'A' && c <= 'Z') { code = letters[c - 'A']; shift = true; }
    else if (c >= '0' && c <= '9') code = digits[c - '0'];
    else {
        for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++)
            if (symbols[i].c == c) { code = symbols[i].code; shift = symbols[i].shift; break; }
    }
    if (code == C_NONE) return false;
    kb_press_code(s, code);
    if (shift) kb_press(s, KB_LSHIFT_COL, KB_LSHIFT_ROW);
    return true;
}

void macro_tick(kb_state_t *out) {
    if (text && time_reached(next)) {
        if (pressed) {                                 // release
            memset(&current, 0, sizeof(current));
            pressed = false;
            next = make_timeout_time_ms(RELEASE_MS);
        } else {
            while (text[pos] && !key_for(text[pos], &current)) pos++;   // skip unknown
            if (text[pos]) {
                pos++;
                pressed = true;
                next = make_timeout_time_ms(PRESS_MS);
            } else {
                text = 0;                              // done
                memset(&current, 0, sizeof(current));
            }
        }
    }
    *out = current;
}
