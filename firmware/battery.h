// CBM2 Keyboard – supply voltage (LiPo battery or USB)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
#ifndef CBM2_BATTERY_H
#define CBM2_BATTERY_H

#include <stdbool.h>

/** Measures VSYS. usb = powered via USB. Returns VSYS in volts. */
float battery_vsys(bool *usb);
/** Text to type, e.g. "akku 3.92v 80%", "usb 4.78v" or "kabel 4.70v"
 *  (lower case = no SHIFT on the C64). cable = powered via the cable from the Pico64. */
const char *battery_text(bool cable);

#endif
