# Änderungen / Changelog

CBM2 Keyboard. Neueste Version zuerst.

## v0.5.0 – 2026-11-24

### Deutsch

- **Neu: C64 Keyboard 0.1.0** – dieselbe Platine mit einer originalen C64-Tastatur (Stecker umgedreht an J1), eigene Firmware `c64-keyboard_pico2w` / `c64-keyboard_rp2040`. Kabel oder Bluetooth zum Pico64, USB-Tastatur (RESTORE + F-Taste), Akku. CTRL + C= + F1 tippt Version und Versorgung. Getestet mit Pico 2 W (Bluetooth, Kabel) und YD-RP2040 (Kabel).
- CBM2-Firmware unverändert 0.4.4 (liegt dem Release wieder bei). Gleiche Quellen, Bauoption `-DKEYBOARD=c64`.

### English

- **New: C64 Keyboard 0.1.0** – the same PCB with an original C64 keyboard (plug turned round on J1), own firmware `c64-keyboard_pico2w` / `c64-keyboard_rp2040`. Cable or Bluetooth to the Pico64, USB keyboard (RESTORE + F key), battery. CTRL + C= + F1 types version and power supply. Tested with Pico 2 W (Bluetooth, cable) and YD-RP2040 (cable).
- CBM2 firmware unchanged at 0.4.4 (attached again). Same sources, build option `-DKEYBOARD=c64`.

## v0.4.4 – 2026-11-07

Erste Veröffentlichung / first public release.

### Deutsch

- CBM-II-Tastatur (500/600/700, US/ASCII) am C64 über den [Pico64 Keyboard](https://github.com/do2mad/pico64-keyboard) – ohne eigenes Kernal. Umsetzung nach Zeichen, F9/F10 als Makros (`LOAD"*",8,1` / `RUN`), CE = RESTORE.
- **Kabel** zum Pico64 (JST-XH 4-polig, 115200 Baud, etwa 3–5 ms Verzögerung, Strom vom C64) – braucht Pico64 ab 0.13.1.
- **Bluetooth** zum Pico64 / BT-64 / Blue-64 (nur Pico 2 W), ohne Kopplung.
- **USB-Tastatur** am Computer (MiSTer, PC/Mac deutsch/US, Auswahl mit CE + F-Taste) und USB-Konsole.
- **Akkubetrieb** mit Lademodul 4056, Stromsparen; **C= + F10** tippt Version und Versorgung.
- Zwei Firmware-Dateien: `cbm2-keyboard_pico2w` (Pico 2 W) und `cbm2-keyboard_rp2040` (YD-RP2040 / Pico 1, nur Kabel; Akkuspannung über Teiler an GP26, am Kabel keine Spannungsanzeige).
- Platine v1.0 (Gerber, Stückliste, Fotos in `hardware/`). Nachtrag 07.11.2026: Gerber mit Ausschnitt unter der Pico-Antenne, zwei Leitungen (GP17, GP21) mit mehr Abstand dazu verlegt, überarbeiteter Bestückungsdruck.

### English

- CBM-II keyboard (500/600/700, US/ASCII) on the C64 through the [Pico64 Keyboard](https://github.com/do2mad/pico64-keyboard) – no custom kernal. Translation by character, F9/F10 as macros (`LOAD"*",8,1` / `RUN`), CE = RESTORE.
- **Cable** to the Pico64 (JST-XH 4-pin, 115200 baud, about 3–5 ms latency, power from the C64) – needs Pico64 0.13.1 or newer.
- **Bluetooth** to the Pico64 / BT-64 / Blue-64 (Pico 2 W only), no pairing.
- **USB keyboard** on a computer (MiSTer, PC/Mac German/US, chosen with CE + F key) and USB console.
- **Battery operation** with a 4056 charger module, power saving; **C= + F10** types version and power supply.
- Two firmware files: `cbm2-keyboard_pico2w` (Pico 2 W) and `cbm2-keyboard_rp2040` (YD-RP2040 / Pico 1, cable only; battery voltage through a divider on GP26, no voltage readout on the cable).
- PCB v1.0 (Gerber, parts list, photos in `hardware/`). Update 2026-11-07: Gerber with a cut-out under the Pico antenna, two traces (GP17, GP21) rerouted with more clearance to it, reworked silkscreen.
