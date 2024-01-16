cmake_minimum_required(VERSION 3.14)

set(PICO_BOARD, "pico_w")

# Include app source code file(s)
# macro files don't need to be included
add_executable(bluetooth_test
	${CMAKE_SOURCE_DIR}/source/bluetooth_test/main.c
)

# Link to built libraries
target_link_libraries(bluetooth_test LINK_PUBLIC pico_stdlib hardware_gpio)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(bluetooth_test 1)
pico_enable_stdio_uart(bluetooth_test 1)

# Enable extra build products
pico_add_extra_outputs(bluetooth_test)
