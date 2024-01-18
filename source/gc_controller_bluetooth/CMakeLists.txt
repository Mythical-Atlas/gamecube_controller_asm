add_executable(gc_controller_sim
	${CMAKE_SOURCE_DIR}/source/gc_controller_sim/main.c
	${CMAKE_SOURCE_DIR}/source/gc_controller_sim/main.S
	${CMAKE_SOURCE_DIR}/source/gc_controller_sim/bt_hid.c
	${COMMON_SRC_DIRECTORY}/sdk_inlines.c
)

target_include_directories(gc_controller_sim PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
)

# Link to built libraries
target_link_libraries(gc_controller_sim
    pico_stdlib
    hardware_gpio
	pico_cyw43_arch_none
        pico_btstack_classic
        pico_btstack_ble
        pico_btstack_cyw43
	pico_multicore
)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(gc_controller_sim 0)
pico_enable_stdio_uart(gc_controller_sim 1)

# Enable extra build products
pico_add_extra_outputs(gc_controller_sim)
