# Änderungen / Changelog

CBM2 Keyboard. Neueste Version zuerst.

## v0.4.4 – 2026-11-07

Erste Veröffentlichung / first public release.

### Deutsch

- CBM-II-Tastatur (500/600/700, US/ASCII) am C64 über den [Pico64 Keyboard](https://github.com/do2mad/pico64-keyboard) – ohne eigenes Kernal. Umsetzung nach Zeichen, F9/F10 als Makros (`LOAD"*",8,1` / `RUN`), CE = RESTORE.
- **Kabel** zum Pico64 (JST-XH 4-polig, 115200 Baud, etwa 3–5 ms Verzögerung, Strom vom C64) – braucht Pico64 ab 0.13.1.
- **Bluetooth** zum Pico64 / BT-64 / Blue-64 (nur Pico 2 W), ohne Kopplung.
- **USB-Tastatur** am Computer (MiSTer, PC/Mac deutsch/US, Auswahl mit CE + F-Taste) und USB-Konsole.
- **Akkubetrieb** mit Lademodul 4056, Stromsparen; **C= + F10** tippt Version und Versorgung.
- Zwei Firmware-Dateien: `cbm2-keyboard_pico2w` (Pico 2 W) und `cbm2-keyboard_rp2040` (YD-RP2040 / Pico 1, nur Kabel; Akkuspannung über Teiler an GP26, am Kabel keine Spannungsanzeige).
- Platine v1.0 (Gerber, Stückliste, Fotos in `hardware/`).

### English

- CBM-II keyboard (500/600/700, US/ASCII) on the C64 through the [Pico64 Keyboard](https://github.com/do2mad/pico64-keyboard) – no custom kernal. Translation by character, F9/F10 as macros (`LOAD"*",8,1` / `RUN`), CE = RESTORE.
- **Cable** to the Pico64 (JST-XH 4-pin, 115200 baud, about 3–5 ms latency, power from the C64) – needs Pico64 0.13.1 or newer.
- **Bluetooth** to the Pico64 / BT-64 / Blue-64 (Pico 2 W only), no pairing.
- **USB keyboard** on a computer (MiSTer, PC/Mac German/US, chosen with CE + F key) and USB console.
- **Battery operation** with a 4056 charger module, power saving; **C= + F10** types version and power supply.
- Two firmware files: `cbm2-keyboard_pico2w` (Pico 2 W) and `cbm2-keyboard_rp2040` (YD-RP2040 / Pico 1, cable only; battery voltage through a divider on GP26, no voltage readout on the cable).
- PCB v1.0 (Gerber, parts list, photos in `hardware/`).
