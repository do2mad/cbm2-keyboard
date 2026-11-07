// CBM2 Keyboard – types short texts (macros) on the C64 keyboard
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_MACRO_H
#define CBM2_MACRO_H

#include <stdbool.h>
#include "kb.h"

/** Start typing text: lower case = key without SHIFT, \r = RETURN, digits,
 *  space and " * , . : ; / = + - @ ! $ % ( ). At most 47 characters. */
bool macro_start(const char *text);
bool macro_busy(void);
/** Call every 2 ms: fills *out with the key state to show now. */
void macro_tick(kb_state_t *out);

#endif
