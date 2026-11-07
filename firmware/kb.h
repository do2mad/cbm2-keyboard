// CBM2 Keyboard – C64 keyboard state and matrix codes
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_KB_H
#define CBM2_KB_H

#include <stdint.h>
#include <stdbool.h>

// Complete C64 keyboard state: cols[n] = PB rows pressed in PA column n
typedef struct {
    uint8_t cols[8];
    bool    restore;
} kb_state_t;

// C64 matrix codes (PA * 8 + PB)
enum {
    C_DEL = 0x00, C_RETURN = 0x01, C_CRSR_LR = 0x02, C_F7 = 0x03, C_F1 = 0x04, C_F3 = 0x05, C_F5 = 0x06, C_CRSR_UD = 0x07,
    C_3 = 0x08, C_W = 0x09, C_A = 0x0A, C_4 = 0x0B, C_Z = 0x0C, C_S = 0x0D, C_E = 0x0E, C_LSHIFT = 0x0F,
    C_5 = 0x10, C_R = 0x11, C_D = 0x12, C_6 = 0x13, C_C = 0x14, C_F = 0x15, C_T = 0x16, C_X = 0x17,
    C_7 = 0x18, C_Y = 0x19, C_G = 0x1A, C_8 = 0x1B, C_B = 0x1C, C_H = 0x1D, C_U = 0x1E, C_V = 0x1F,
    C_9 = 0x20, C_I = 0x21, C_J = 0x22, C_0 = 0x23, C_M = 0x24, C_K = 0x25, C_O = 0x26, C_N = 0x27,
    C_PLUS = 0x28, C_P = 0x29, C_L = 0x2A, C_MINUS = 0x2B, C_PERIOD = 0x2C, C_COLON = 0x2D, C_AT = 0x2E, C_COMMA = 0x2F,
    C_POUND = 0x30, C_STAR = 0x31, C_SEMI = 0x32, C_HOME = 0x33, C_RSHIFT = 0x34, C_EQUAL = 0x35, C_UPARROW = 0x36, C_SLASH = 0x37,
    C_1 = 0x38, C_LEFTARROW = 0x39, C_CTRL = 0x3A, C_2 = 0x3B, C_SPACE = 0x3C, C_CMDR = 0x3D, C_Q = 0x3E, C_STOP = 0x3F,
    C_NONE = 0xFF,
};

// C64 matrix positions (PA column, PB row) of the modifier keys
#define KB_LSHIFT_COL 1
#define KB_LSHIFT_ROW 7
#define KB_RSHIFT_COL 6
#define KB_RSHIFT_ROW 4
#define KB_CMDR_COL   7
#define KB_CMDR_ROW   5
#define KB_CTRL_COL   7
#define KB_CTRL_ROW   2

static inline void kb_press(kb_state_t *s, uint8_t col, uint8_t row) {
    s->cols[col & 7] |= (uint8_t)(1u << (row & 7));
}

/** Press the key with matrix code PA*8+PB (0..63). */
static inline void kb_press_code(kb_state_t *s, uint8_t code) {
    if (code < 64) kb_press(s, code >> 3, code & 7);
}

static inline bool kb_held(const kb_state_t *s, uint8_t code) {
    return code < 64 && (s->cols[code >> 3] & (1u << (code & 7)));
}

static inline bool kb_empty(const kb_state_t *s) {
    for (int i = 0; i < 8; i++) if (s->cols[i]) return false;
    return !s->restore;
}

#endif
