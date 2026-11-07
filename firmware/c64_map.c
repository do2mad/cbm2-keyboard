// CBM2 Keyboard – CBM-II keys -> C64 keyboard
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// Symbolic mapping: what is printed on the CBM-II key arrives on the C64
// (SHIFT+2 = @, SHIFT+; = :, ' = SHIFT+7 on the C64 ...). Letters keep SHIFT
// (graphic characters). With CTRL or C= held the digits stay digits with SHIFT
// unchanged (colours, RVS ON/OFF).
//
//   F1..F8          F1..F8 (F2 = SHIFT+F1 ...)
//   F9 / F10        macros  LOAD"*",8,1 RETURN  /  RUN RETURN   (macro.c)
//   cursor keys     CRSR (up / left with SHIFT)
//   CLR/HOME        CLR/HOME            INS/DEL    INST/DEL
//   OFF/RVS         CTRL+9 (RVS ON), with SHIFT CTRL+0 (RVS OFF)
//   NORM/GRAPH      SHIFT+C= (switch character set)
//   RUN/STOP        RUN/STOP            ESC        <- (arrow left)
//   <- / pound      <-, with SHIFT pound
//   PI              PI (SHIFT + arrow up)
//   RETURN, ENTER   RETURN              TAB        nothing (yet)
//   keypad          digits and + - * / . ?   00 = macro "00"
//   keypad CE       RESTORE
//   SHIFT, SHIFT LOCK, CTRL, C=  the same keys on the C64

#include "c64_map.h"

#define K(r, c) CBM_KEY(r, c)

enum { SH_KEEP = 0, SH_ON = 1, SH_OFF = 2 };
enum { X_NONE = 0, X_RESTORE, X_RVS, X_GRAPH, X_MOD };

typedef struct { uint8_t code, shift; } ck_t;
typedef struct {
    uint8_t key;           // CBM-II key
    ck_t    plain, shifted;
    uint8_t extra;
} entry_t;

#define E(key, c1, s1, c2, s2) { (key), { (c1), (s1) }, { (c2), (s2) }, X_NONE }
#define L(key, c)              { (key), { (c), SH_KEEP }, { (c), SH_KEEP }, X_NONE }
#define X(key, x)              { (key), { C_NONE, SH_KEEP }, { C_NONE, SH_KEEP }, (x) }

static const entry_t table[] = {
    // row C0
    E(K(0,0), C_F1, SH_OFF, C_F1, SH_OFF), E(K(0,1), C_F1, SH_ON, C_F1, SH_ON),
    E(K(0,2), C_F3, SH_OFF, C_F3, SH_OFF), E(K(0,3), C_F3, SH_ON, C_F3, SH_ON),
    E(K(0,4), C_F5, SH_OFF, C_F5, SH_OFF), E(K(0,5), C_F5, SH_ON, C_F5, SH_ON),
    E(K(0,6), C_F7, SH_OFF, C_F7, SH_OFF), E(K(0,7), C_F7, SH_ON, C_F7, SH_ON),
    // F9, F10: macros (handled in main.c)
    E(K(0,10), C_CRSR_UD, SH_OFF, C_CRSR_UD, SH_OFF),     // down
    E(K(0,11), C_CRSR_UD, SH_ON,  C_CRSR_UD, SH_ON),      // up
    L(K(0,12), C_HOME),                                    // CLR/HOME
    X(K(0,13), X_RVS),                                     // OFF/RVS
    X(K(0,14), X_GRAPH),                                   // NORM/GRAPH
    L(K(0,15), C_STOP),                                    // RUN/STOP
    // row C1
    L(K(1,0), C_LEFTARROW),                                // ESC -> <-
    E(K(1,1), C_1, SH_OFF, C_1, SH_ON),                    // 1 !
    E(K(1,2), C_2, SH_OFF, C_AT, SH_OFF),                  // 2 @
    E(K(1,3), C_3, SH_OFF, C_3, SH_ON),                    // 3 #
    E(K(1,4), C_4, SH_OFF, C_4, SH_ON),                    // 4 $
    E(K(1,5), C_5, SH_OFF, C_5, SH_ON),                    // 5 %
    E(K(1,6), C_7, SH_OFF, C_6, SH_ON),                    // 7 &
    E(K(1,7), C_8, SH_OFF, C_STAR, SH_OFF),                // 8 *
    E(K(1,8), C_9, SH_OFF, C_8, SH_ON),                    // 9 (
    E(K(1,9), C_0, SH_OFF, C_9, SH_ON),                    // 0 )
    E(K(1,10), C_EQUAL, SH_OFF, C_PLUS, SH_OFF),           // = +
    E(K(1,11), C_CRSR_LR, SH_ON, C_CRSR_LR, SH_ON),        // cursor left
    E(K(1,12), C_SLASH, SH_ON, C_SLASH, SH_ON),            // keypad ?
    X(K(1,13), X_RESTORE),                                 // keypad CE
    E(K(1,14), C_STAR, SH_OFF, C_STAR, SH_OFF),            // keypad *
    E(K(1,15), C_SLASH, SH_OFF, C_SLASH, SH_OFF),          // keypad /
    // row C2
    // TAB (2,0): nothing yet
    L(K(2,1), C_Q), L(K(2,2), C_W), L(K(2,3), C_E), L(K(2,4), C_R),
    E(K(2,5), C_6, SH_OFF, C_UPARROW, SH_OFF),             // 6 ^ (arrow up)
    L(K(2,6), C_U), L(K(2,7), C_I), L(K(2,8), C_O),
    E(K(2,9), C_MINUS, SH_OFF, C_MINUS, SH_OFF),           // -
    E(K(2,10), C_LEFTARROW, SH_OFF, C_POUND, SH_OFF),      // <- pound
    E(K(2,11), C_CRSR_LR, SH_OFF, C_CRSR_LR, SH_OFF),      // cursor right
    E(K(2,12), C_7, SH_OFF, C_7, SH_OFF),                  // keypad 7 8 9 -
    E(K(2,13), C_8, SH_OFF, C_8, SH_OFF),
    E(K(2,14), C_9, SH_OFF, C_9, SH_OFF),
    E(K(2,15), C_MINUS, SH_OFF, C_MINUS, SH_OFF),
    // row C3
    L(K(3,1), C_A), L(K(3,2), C_S), L(K(3,3), C_D), L(K(3,4), C_T), L(K(3,5), C_Y),
    L(K(3,6), C_J), L(K(3,7), C_K), L(K(3,8), C_L), L(K(3,9), C_P),
    E(K(3,10), C_SEMI, SH_ON, C_SEMI, SH_ON),              // ]
    L(K(3,11), C_DEL),                                     // INS/DEL
    E(K(3,12), C_4, SH_OFF, C_4, SH_OFF),                  // keypad 4 5 6 +
    E(K(3,13), C_5, SH_OFF, C_5, SH_OFF),
    E(K(3,14), C_6, SH_OFF, C_6, SH_OFF),
    E(K(3,15), C_PLUS, SH_OFF, C_PLUS, SH_OFF),
    // row C4
    X(K(4,0), X_MOD),                                      // SHIFT, SHIFT LOCK
    L(K(4,1), C_Z), L(K(4,2), C_X), L(K(4,3), C_F), L(K(4,4), C_G), L(K(4,5), C_H), L(K(4,6), C_M),
    E(K(4,7), C_COMMA, SH_OFF, C_COMMA, SH_ON),            // , <
    E(K(4,8), C_SEMI, SH_OFF, C_COLON, SH_OFF),            // ; :
    E(K(4,9), C_COLON, SH_ON, C_COLON, SH_ON),             // [
    L(K(4,10), C_RETURN),                                  // RETURN
    X(K(4,11), X_MOD),                                     // C=
    E(K(4,12), C_1, SH_OFF, C_1, SH_OFF),                  // keypad 1 2 3
    E(K(4,13), C_2, SH_OFF, C_2, SH_OFF),
    E(K(4,14), C_3, SH_OFF, C_3, SH_OFF),
    L(K(4,15), C_RETURN),                                  // ENTER
    // row C5
    X(K(5,0), X_MOD),                                      // CTRL
    L(K(5,2), C_C), L(K(5,3), C_V), L(K(5,4), C_B), L(K(5,5), C_N),
    L(K(5,6), C_SPACE),
    E(K(5,7), C_PERIOD, SH_OFF, C_PERIOD, SH_ON),          // . >
    E(K(5,8), C_SLASH, SH_OFF, C_SLASH, SH_ON),            // / ?
    E(K(5,9), C_7, SH_ON, C_2, SH_ON),                     // ' "
    E(K(5,10), C_UPARROW, SH_ON, C_UPARROW, SH_ON),        // PI
    E(K(5,12), C_0, SH_OFF, C_0, SH_OFF),                  // keypad 0 .
    E(K(5,13), C_PERIOD, SH_OFF, C_PERIOD, SH_OFF),
    // keypad 00: macro
};

#define KEY_SHIFT K(4,0)
#define KEY_CMDR  K(4,11)
#define KEY_CTRL  K(5,0)

static const entry_t *find(uint8_t key) {
    for (unsigned i = 0; i < sizeof(table) / sizeof(table[0]); i++)
        if (table[i].key == key) return &table[i];
    return 0;
}

// the unshifted digit keys of the main block: with CTRL / C= they stay digits
static bool is_digit_key(uint8_t key) {
    return (key >= K(1,1) && key <= K(1,9)) || key == K(2,5);
}

void c64_map(const cbm_keys_t *k, kb_state_t *out) {
    kb_state_t s = {0};
    bool shift = false, ctrl = false, cmdr = false;
    for (int i = 0; i < k->n; i++) {
        if (k->held[i] == KEY_SHIFT) shift = true;
        if (k->held[i] == KEY_CTRL)  ctrl = true;
        if (k->held[i] == KEY_CMDR)  cmdr = true;
    }
    int override = SH_KEEP;            // the last pressed key decides about SHIFT

    for (int i = 0; i < k->n; i++) {
        const entry_t *e = find(k->held[i]);
        if (!e) continue;
        switch (e->extra) {
        case X_RESTORE: s.restore = true; continue;
        case X_RVS:                                   // CTRL+9 / CTRL+0
            kb_press_code(&s, shift ? C_0 : C_9);
            kb_press(&s, KB_CTRL_COL, KB_CTRL_ROW);
            override = SH_OFF;
            continue;
        case X_GRAPH:                                 // SHIFT+C=
            kb_press(&s, KB_CMDR_COL, KB_CMDR_ROW);
            override = SH_ON;
            continue;
        case X_MOD:
            continue;
        default:
            break;
        }
        ck_t c = shift ? e->shifted : e->plain;
        if ((ctrl || cmdr) && is_digit_key(e->key)) c = (ck_t){ e->plain.code, SH_KEEP };
        if (c.code == C_NONE) continue;
        kb_press_code(&s, c.code);
        if (c.shift != SH_KEEP) override = c.shift;
    }

    if (ctrl) kb_press(&s, KB_CTRL_COL, KB_CTRL_ROW);
    if (cmdr) kb_press(&s, KB_CMDR_COL, KB_CMDR_ROW);
    bool c64_shift = shift;
    if (override == SH_ON) c64_shift = true;
    else if (override == SH_OFF) c64_shift = false;
    if (c64_shift) kb_press(&s, KB_LSHIFT_COL, KB_LSHIFT_ROW);
    *out = s;
}

const char *c64_macro(uint8_t key) {
    switch (key) {
    case K(0,8):  return "load\"*\",8,1\r";      // F9
    case K(0,9):  return "run\r";                // F10
    case K(5,14): return "00";                   // keypad 00
    default:      return 0;
    }
}
