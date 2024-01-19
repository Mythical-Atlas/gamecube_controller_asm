add_executable(gc_controller_bluetooth
	${CMAKE_SOURCE_DIR}/source/gc_controller_bluetooth/main.c
	${CMAKE_SOURCE_DIR}/source/gc_controller_bluetooth/bt_hid.c
)

target_include_directories(gc_controller_bluetooth PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
)

# Link to built libraries
target_link_libraries(gc_controller_bluetooth
    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_dma
	pico_cyw43_arch_none
        pico_btstack_classic
        pico_btstack_ble
        pico_btstack_cyw43
	pico_multicore
)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(gc_controller_bluetooth 0)
pico_enable_stdio_uart(gc_controller_bluetooth 1)

# Enable extra build products
pico_add_extra_outputs(gc_controller_bluetooth)
