# CBM2 Keyboard – CBM-II keyboard for the C64 (via Pico64 Keyboard) and as USB keyboard
# Build: mkdir build && cd build && cmake .. && make -j            (Pico 2 W)
#        mkdir build-rp2040 && cd build-rp2040 && cmake -DPICO_BOARD=pico .. && make -j   (RP2040)
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(cbm2_keyboard C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

add_executable(cbm2_keyboard
    main.c
    cbm2_scan.c
    c64_map.c
    macro.c
    usb_kbd.c
    usb_out.c
    battery.c
    wire.c
)

target_include_directories(cbm2_keyboard PRIVATE ${CMAKE_CURRENT_LIST_DIR})

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
