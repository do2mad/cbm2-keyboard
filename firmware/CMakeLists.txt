# CBM2 Keyboard – CBM-II keyboard for the C64 (via Pico64 Keyboard) and as USB keyboard
# Build: mkdir build && cd build && cmake .. && make -j            (Pico 2 W)
#        add -DKEYBOARD=c64 for an original C64 keyboard on the same PCB
#        mkdir build-rp2040 && cd build-rp2040 && cmake -DPICO_BOARD=pico .. && make -j   (RP2040)
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(cbm2_keyboard C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

# KEYBOARD=cbm2 (default): CBM-II keyboard. KEYBOARD=c64: original C64 keyboard on the
# same PCB (plug turned round at the end of J1, see c64_scan.c) -> c64_keyboard.uf2
set(KEYBOARD cbm2 CACHE STRING "cbm2 or c64")
if(KEYBOARD STREQUAL "c64")
    set(KB_SCAN c64_scan.c)
    set(KB_MAP "")
    set(KB_C64 1)
else()
    set(KB_SCAN cbm2_scan.c)
    set(KB_MAP c64_map.c)
    set(KB_C64 0)
endif()

add_executable(cbm2_keyboard
    main.c
    ${KB_SCAN}
    ${KB_MAP}
    macro.c
    usb_kbd.c
    usb_out.c
    battery.c
    wire.c
)

target_include_directories(cbm2_keyboard PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_compile_definitions(cbm2_keyboard PRIVATE KB_C64=${KB_C64})
if(KB_C64)
    set_target_properties(cbm2_keyboard PROPERTIES OUTPUT_NAME c64_keyboard)
endif()

target_link_libraries(cbm2_keyboard
    pico_stdlib
    tinyusb_device       # own USB descriptors (console + keyboard)
    pico_unique_id
    hardware_uart
)

# Pico 2 W (default): Bluetooth to the Pico64 + cable + USB.
# Any other board, e.g. cmake -DPICO_BOARD=pico .. for RP2040 boards like the
# YD-RP2040: no radio - cable to the Pico64 + USB only.
if(PICO_CYW43_SUPPORTED)
    target_sources(cbm2_keyboard PRIVATE ble_link.c)
    target_compile_definitions(cbm2_keyboard PRIVATE CBM2_BT=1)
    target_link_libraries(cbm2_keyboard
        pico_btstack_ble
        pico_btstack_cyw43
        pico_cyw43_arch_none
        hardware_adc
    )
else()
    target_sources(cbm2_keyboard PRIVATE ble_none.c)
    target_compile_definitions(cbm2_keyboard PRIVATE CBM2_BT=0)
    target_link_libraries(cbm2_keyboard hardware_flash hardware_sync hardware_adc)
endif()

# USB serial console (115200); the descriptors come from usb_kbd.c, stdio uses its CDC port
pico_enable_stdio_usb(cbm2_keyboard 1)
pico_enable_stdio_uart(cbm2_keyboard 0)

pico_add_extra_outputs(cbm2_keyboard)
