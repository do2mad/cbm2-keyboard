// CBM2 Keyboard – USB keyboard output (PC, Mac, MiSTer)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_USB_OUT_H
#define CBM2_USB_OUT_H

#include <stdbool.h>
#include <stddef.h>
#include "kb.h"

void usb_out_init(void);
/** New C64 key state; active = false sends "nothing pressed". Call every tick. */
void usb_out_update(const kb_state_t *s, bool active);
const char *usb_out_mode_name(void);
/** Console commands "!u" (status) and "!u0".."!u4" (output mode). */
bool usb_out_command(const char *cmd, size_t len);

#endif
