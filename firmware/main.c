// CBM2 Keyboard – CBM-II keyboard (500/600/700) for the C64 and as USB keyboard
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// A Raspberry Pi Pico 2 W scans the CBM-II keyboard (cbm2_scan.c), turns the keys
// into C64 keys (c64_map.c, macro.c) and sends them
//   - via Bluetooth LE to a Pico64 Keyboard in the C64 (or a BT-64 / Blue-64),
//     like the Blue-64 Keyboard app (ble_link.c), and
//   - via a cable to the Pico64 Keyboard instead of Bluetooth (wire.c; Bluetooth is
//     off while the cable is plugged in, the cable also powers the Pico),
//   - via USB as a keyboard (PC, Mac, MiSTer), when a computer is connected and
//     no Bluetooth link / cable is in use (usb_out.c).
//
// USB serial console (115200): key log and commands
//   d     key log on/off           !s   status
//   !r    Bluetooth: disconnect and search again
//   !u    USB keyboard output, !u0..!u4 choose it (see usb_out.c)
//   !v    supply voltage
//
// C= + F10 types firmware version and supply voltage, e.g. "cbm2 v0.3.1 akku 3.92v 80%"
// (on the C64 screen, or on the computer via USB).
//
// Everything runs in the BTstack run loop on core 0; between timers the CPU sleeps.
// Power saving (battery): slower scanning when idle, Bluetooth search with low duty
// cycle after a minute, LED only flashes. (v0.3.0 also ran at 64 MHz - the Pico did
// not start any more, so v0.3.2 is back to the default 150 MHz.)

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pico/stdlib.h"
#if CBM2_BT
#include "pico/cyw43_arch.h"
#endif
#include "hardware/clocks.h"
#if CBM2_BT
#include "btstack.h"
#endif

#include "kb.h"
#include "cbm2_scan.h"
#include "c64_map.h"
#include "macro.h"
#include "ble_link.h"
#include "usb_kbd.h"
#include "usb_out.h"
#include "battery.h"
#include "wire.h"

#define CBM2_VERSION "0.4.4"
// The timer runs every 1 ms (USB must be served often, also while a computer sets
// us up). The keyboard is scanned every 1 ms with the cable to the Pico64 or a
// computer on USB (fast, e.g. for games); otherwise (battery, Bluetooth) every
// ACTIVE_MS while keys are used and every IDLE_MS after IDLE_AFTER_MS without keys.
// Debouncing needs 3 equal scans.
#define TICK_MS       1
#define FAST_SCAN_MS  1
#define ACTIVE_MS     4
#define IDLE_MS       10
#define IDLE_AFTER_MS 3000

#if CBM2_BT
static btstack_timer_source_t tick, led_timer;
#endif
static bool key_log = true;
static char line[32];
static size_t line_len;

static void log_keys(const kb_state_t *s) {
    const cbm_keys_t *k = cbm_keys();
    printf("%8lu ms  ", (unsigned long)to_ms_since_boot(get_absolute_time()));
    if (k->n == 0) printf("(none)");
    for (int i = 0; i < k->n; i++) printf("%s%s", i ? " + " : "", cbm_key_name(k->held[i]));
    printf("  -> C64 ");
    for (int i = 0; i < 8; i++) printf("%02x", s->cols[i]);
    printf("%s\n", s->restore ? " +RESTORE" : "");
}

// typed with C= + F10, e.g. "cbm2 v0.3.1 akku 3.92v 80%"
static const char *info_text(void) {
    static char buf[48];
    snprintf(buf, sizeof(buf), "cbm2 v" CBM2_VERSION " %s", battery_text(wire_connected()));
    return buf;
}

static void status(void) {
    printf("CBM2 Keyboard v" CBM2_VERSION " (%lu MHz)\n", (unsigned long)(clock_get_hz(clk_sys) / 1000000));
    printf("cable to the Pico64: %s\n", wire_connected() ? "plugged in" : "no");
    ble_link_status();
    printf("USB: %s, output %s%s\n", usb_kbd_mounted() ? "connected" : "not connected",
           usb_out_mode_name(), (ble_link_ready() || wire_connected()) ? " (paused while the C64 link is up)" : "");
    printf("supply: %s\n", battery_text(wire_connected()));
    printf("key log %s\n", key_log ? "on" : "off");
}

static void console(void) {
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\r' || c == '\n') {
            if (line_len == 0) continue;
            line[line_len] = 0;
            for (size_t i = 0; i < line_len; i++) line[i] = (char)tolower((unsigned char)line[i]);
            if (!strcmp(line, "d"))       { key_log = !key_log; printf("key log %s\n", key_log ? "on" : "off"); }
            else if (!strcmp(line, "!s")) status();
            else if (!strcmp(line, "!r")) ble_link_reconnect();
            else if (!strcmp(line, "!v")) printf("supply: %s\n", battery_text(wire_connected()));
            else if (!usb_out_command(line, line_len))
                printf("commands: d (key log), !s (status), !r (Bluetooth search again), !v (voltage), !u / !u0..!u4 (USB output)\n");
            line_len = 0;
        } else if ((c == 8 || c == 127) && line_len > 0) {
            line_len--;
        } else if (c >= 32 && c < 127 && line_len < sizeof(line) - 1) {
            line[line_len++] = (char)c;
        }
    }
}

static void tick_body(void) {
    static unsigned n;
    static uint8_t prev_held[CBM_MAX_HELD];
    static int prev_n;
    static kb_state_t last_out;
    static bool was_wire;

    static absolute_time_t active_until;
    static bool was_usb, was_ble;

    usb_kbd_task();
    wire_tick(&last_out);
    bool wire = wire_connected();
    if (wire != was_wire) {
        printf(wire ? "cable to the Pico64 plugged in\n" : "cable to the Pico64 unplugged\n");
        ble_link_enable(!wire);
        was_wire = wire;
    }
    bool usb = usb_kbd_mounted();
    bool ble = ble_link_ready() || wire;      // a link to the C64 is up
    if (usb != was_usb || (usb && ble != was_ble)) {
        if (!usb)     printf("USB: no computer\n");
        else if (ble) printf("USB: computer connected - keyboard paused while the C64 link (Bluetooth / cable) is up\n");
        else          printf("USB: computer connected - USB keyboard active (%s)\n", usb_out_mode_name());
        was_usb = usb;
        was_ble = ble;
    }
    uint32_t period = (usb || wire) ? FAST_SCAN_MS : (time_reached(active_until) ? IDLE_MS : ACTIVE_MS);
    if (++n >= period) {
        n = 0;
        bool changed = cbm_scan();
        if (changed || cbm_keys()->n > 0 || macro_busy())
            active_until = make_timeout_time_ms(IDLE_AFTER_MS);
        if (changed) {
            // macro keys: start on press
            const cbm_keys_t *k = cbm_keys();
            for (int i = 0; i < k->n; i++) {
                bool was = false;
                for (int j = 0; j < prev_n; j++) if (prev_held[j] == k->held[i]) was = true;
                const char *m = NULL;
                if (!was) {
                    if (k->held[i] == CBM_KEY(0, 9) && cbm_is_held(CBM_KEY(4, 11)))   // C= + F10
                        m = info_text();
                    else
                        m = c64_macro(k->held[i]);
                }
                if (m && macro_start(m) && key_log) printf("macro: %s\n", cbm_key_name(k->held[i]));
            }
            memcpy(prev_held, k->held, sizeof(prev_held));
            prev_n = k->n;
        }
        kb_state_t s;
        if (macro_busy()) macro_tick(&s);
        else              c64_map(cbm_keys(), &s);
        if (changed && key_log && !macro_busy()) log_keys(&s);
        if (memcmp(&s, &last_out, sizeof(s)) != 0) {
            last_out = s;
            ble_link_send(&s);
        }
        ble_link_tick();
        usb_out_update(&s, usb && !ble);
        console();
    }
}

// LED (short flashes to save power): searching / no link = flash every second,
// connected to the C64 (Bluetooth or cable) = flash every 4 seconds
#if !CBM2_BT
#define LED_PIN 25               // RP2040 boards (Pico, YD-RP2040): LED on GP25
#endif

static uint32_t led_step(void) {
    static bool on;
    on = !on;
#if CBM2_BT
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
#else
    gpio_put(LED_PIN, on);
#endif
    return on ? 60 : ((ble_link_ready() || wire_connected()) ? 4000 : 940);
}

#if CBM2_BT
static void on_tick(btstack_timer_source_t *ts) {
    tick_body();
    btstack_run_loop_set_timer(ts, TICK_MS);
    btstack_run_loop_add_timer(ts);
}

static void led_tick(btstack_timer_source_t *ts) {
    btstack_run_loop_set_timer(ts, led_step());
    btstack_run_loop_add_timer(ts);
}

int main(void) {
    cbm_scan_init();
    wire_init();

    // Wireless chip and Bluetooth first: loading their firmware blocks for a while.
    // USB is switched on afterwards, right before the run loop serves it - otherwise
    // the computer tries to set us up while nobody answers and gives up (v0.1-v0.3.3).
    if (cyw43_arch_init()) return -1;

    usb_out_init();
    ble_link_init();

    tick.process = &on_tick;
    btstack_run_loop_set_timer(&tick, TICK_MS);
    btstack_run_loop_add_timer(&tick);
    led_timer.process = &led_tick;
    btstack_run_loop_set_timer(&led_timer, 400);
    btstack_run_loop_add_timer(&led_timer);

    hci_power_control(HCI_POWER_ON);

    usb_kbd_init();              // own USB descriptors: console + keyboard
    stdio_init_all();
    printf("CBM2 Keyboard v" CBM2_VERSION " starting\n");

    btstack_run_loop_execute();  // does not return
    return 0;
}
#else
// RP2040 without radio: cable to the Pico64 and USB only, simple 1 ms main loop
int main(void) {
    cbm_scan_init();
    wire_init();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    usb_out_init();
    usb_kbd_init();
    stdio_init_all();
    printf("CBM2 Keyboard v" CBM2_VERSION " (RP2040, cable only) starting\n");

    absolute_time_t next_tick = get_absolute_time(), next_led = next_tick;
    for (;;) {
        if (time_reached(next_tick)) {
            next_tick = delayed_by_ms(next_tick, TICK_MS);
            tick_body();
        }
        if (time_reached(next_led)) next_led = make_timeout_time_ms(led_step());
        best_effort_wfe_or_timeout(next_tick);   // sleep until the next tick (or an interrupt)
    }
}
#endif
