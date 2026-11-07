// CBM2 Keyboard – supply voltage (LiPo battery or USB)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// VSYS is measured on GP29 (ADC 3, divider 1:3). On the Pico 2 W this pin is shared
// with the wireless chip - the measurement follows the pico-examples
// (adc/read_vsys/power_status.c, Copyright (c) 2023 Raspberry Pi (Trading) Ltd.,
// BSD-3-Clause).
//
// Battery wiring: LiPo + -> Schottky diode (1N5817) -> VSYS, so the battery voltage
// is VSYS + DIODE_DROP. The percentage is a rough LiPo curve (no load correction).

#include "battery.h"

#if CBM2_BT

#include <stdio.h>
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"

#define DIODE_DROP   0.30f
#define SAMPLES      8
#define FIRST_ADC    26

float battery_vsys(bool *usb) {
    cyw43_thread_enter();
    bool vbus = cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);   // also wakes the chip up
    adc_init();
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - FIRST_ADC);
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    int ignore = SAMPLES;
    while (!adc_fifo_is_empty() || ignore-- > 0) (void)adc_fifo_get_blocking();
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) sum += adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
    cyw43_thread_exit();
    if (usb) *usb = vbus;
    return (float)sum / SAMPLES * 3.0f * 3.3f / 4096.0f;
}

static int lipo_percent(float v) {
    static const float curve[][2] = {          // volts, percent (resting LiPo)
        {4.20f, 100}, {4.10f, 90}, {4.00f, 80}, {3.90f, 65}, {3.80f, 50},
        {3.70f, 35}, {3.60f, 20}, {3.50f, 10}, {3.40f, 5}, {3.20f, 0},
    };
    if (v >= curve[0][0]) return 100;
    for (unsigned i = 1; i < sizeof(curve) / sizeof(curve[0]); i++) {
        if (v >= curve[i][0]) {
            float f = (v - curve[i][0]) / (curve[i - 1][0] - curve[i][0]);
            return (int)(curve[i][1] + f * (curve[i - 1][1] - curve[i][1]) + 0.5f);
        }
    }
    return 0;
}

const char *battery_text(bool cable) {
    static char buf[32];
    bool usb;
    float v = battery_vsys(&usb);
    int mv;
    if (usb) {
        mv = (int)(v * 1000.0f + 0.5f);
        snprintf(buf, sizeof(buf), "usb %d.%02dv", mv / 1000, (mv % 1000) / 10);
    } else if (cable) {                                  // 5 V from the Pico64 via the cable
        mv = (int)((v + DIODE_DROP) * 1000.0f + 0.5f);
        snprintf(buf, sizeof(buf), "kabel %d.%02dv", mv / 1000, (mv % 1000) / 10);
    } else {
        float bat = v + DIODE_DROP;
        mv = (int)(bat * 1000.0f + 0.5f);
        snprintf(buf, sizeof(buf), "akku %d.%02dv %d%%", mv / 1000, (mv % 1000) / 10, lipo_percent(bat));
    }
    return buf;
}

#else   // RP2040 boards (no radio): optional divider Vin - 100k - GP26 - 100k - GND

#include <stdio.h>
#include "hardware/adc.h"
#include "usb_kbd.h"

#define DIODE_DROP   0.30f       // battery / cable feed Vin through a 1N5817
#define DIV_PIN      26          // ADC 0
#define DIV_FACTOR   2.0f        // 100k / 100k
#define SAMPLES      8

float battery_vsys(bool *usb) {
    static bool init;
    if (!init) {
        adc_init();
        adc_gpio_init(DIV_PIN);
        init = true;
    }
    adc_select_input(DIV_PIN - 26);
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) sum += adc_read();
    if (usb) *usb = usb_kbd_host();
    return (float)sum / SAMPLES * DIV_FACTOR * 3.3f / 4096.0f;
}

static int lipo_percent(float v) {
    static const float curve[][2] = {
        {4.20f, 100}, {4.10f, 90}, {4.00f, 80}, {3.90f, 65}, {3.80f, 50},
        {3.70f, 35}, {3.60f, 20}, {3.50f, 10}, {3.40f, 5}, {3.20f, 0},
    };
    if (v >= curve[0][0]) return 100;
    for (unsigned i = 1; i < sizeof(curve) / sizeof(curve[0]); i++) {
        if (v >= curve[i][0]) {
            float f = (v - curve[i][0]) / (curve[i - 1][0] - curve[i][0]);
            return (int)(curve[i][1] + f * (curve[i - 1][1] - curve[i][1]) + 0.5f);
        }
    }
    return 0;
}

const char *battery_text(bool cable) {
    static char buf[40];
    bool usb;
    // Cable to the Pico64: powered by the C64, no voltage shown (the divider is
    // only fitted / meaningful for battery operation).
    if (cable)
        return "rp2040 kabel";
    float v = battery_vsys(&usb);
    // Without a divider GP26 floats (reads anything, often near 3.3 V = "6.x V"):
    // only plausible values are shown.
    if (v < 1.0f || v > 5.6f)
        return "rp2040";
    if (usb) {
        int mv = (int)(v * 1000.0f + 0.5f);
        snprintf(buf, sizeof(buf), "rp2040 usb %d.%02dv", mv / 1000, (mv % 1000) / 10);
    } else {
        float bat = v + DIODE_DROP;
        int mv = (int)(bat * 1000.0f + 0.5f);
        snprintf(buf, sizeof(buf), "rp2040 akku %d.%02dv %d%%", mv / 1000, (mv % 1000) / 10, lipo_percent(bat));
    }
    return buf;
}

#endif
