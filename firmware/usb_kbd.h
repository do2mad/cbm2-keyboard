// CBM2 Keyboard – USB device: serial console + USB keyboard
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_USB_KBD_H
#define CBM2_USB_KBD_H

#include <stdint.h>
#include <stdbool.h>

void usb_kbd_init(void);                                 // before stdio_init_all()
void usb_kbd_task(void);                                 // call often (every 1-2 ms)
bool usb_kbd_mounted(void);                              // a computer uses us
bool usb_kbd_host(void);                                 // a computer is connected (bus reset seen)
void usb_kbd_set(uint8_t mods, const uint8_t keys[6]);   // new key state (sent when changed)

#endif
