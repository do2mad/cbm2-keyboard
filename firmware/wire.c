// CBM2 Keyboard – cable link to the Pico64 Keyboard (instead of Bluetooth)
// Copyright (c) 2026 Martin Oswald (1mhz.de) - SPDX-License-Identifier: MIT
//
// Cable (4 wires) CBM2 Pico  ->  Pico64 Keyboard:
//   GP28 (pin 34, UART0 TX) -- 330 R --  GP17 (pin 22, UART0 RX)
//   GP22 (pin 29, detect)   ------------  GND   (the plug pulls it low = cable in)
//   VSYS (pin 39) <- 1N5817 <-----------  VSYS  (pin 39, 5 V of the C64 behind its diode)
//   GND                     ------------  GND
//
// Frame, 115200 baud 8N1 (about 1 ms), sent on every change and every 100 ms (keep-alive, the
// Pico64 releases the keys after 300 ms without a frame):
//   0xC6 0x4B  flags  col0..col7  check      flags bit 3 = RESTORE
//   check = XOR of flags and col0..col7, XOR 0x5A

#include "wire.h"

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define WIRE_UART     uart0
#define PIN_TX        28
#define PIN_DETECT    22
#define BAUD          115200
#define KEEPALIVE_MS  100
#define DETECT_MS     50       // detect line must be stable this long

static bool connected;
static kb_state_t last;
static absolute_time_t next_send;
static uint32_t detect_count;

void wire_init(void) {
    gpio_init(PIN_DETECT);
    gpio_set_dir(PIN_DETECT, GPIO_IN);
    gpio_pull_up(PIN_DETECT);
    uart_init(WIRE_UART, BAUD);
    gpio_set_function(PIN_TX, GPIO_FUNC_UART);
}

static void send(const kb_state_t *s) {
    uint8_t f[12];
    f[0] = 0xC6;
    f[1] = 0x4B;
    f[2] = s->restore ? 0x08 : 0x00;
    memcpy(&f[3], s->cols, 8);
    uint8_t x = 0x5A;
    for (int i = 2; i < 11; i++) x ^= f[i];
    f[11] = x;
    if (uart_is_writable(WIRE_UART)) uart_write_blocking(WIRE_UART, f, sizeof(f));   // FIFO: 32 bytes
}

void wire_tick(const kb_state_t *s) {
    bool plugged = !gpio_get(PIN_DETECT);
    if (plugged != connected) {
        if (++detect_count >= DETECT_MS) { connected = plugged; detect_count = 0; }
    } else {
        detect_count = 0;
    }
    if (!connected) return;
    if (memcmp(s, &last, sizeof(last)) != 0 || time_reached(next_send)) {
        last = *s;
        send(s);
        next_send = make_timeout_time_ms(KEEPALIVE_MS);
    }
}

bool wire_connected(void) { return connected; }
