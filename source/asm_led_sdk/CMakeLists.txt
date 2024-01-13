cmake_minimum_required(VERSION 3.14)

# Include app source code file(s)
add_executable(LED_SDK ${CMAKE_SOURCE_DIR}/source/asm_led_sdk/main.S ${COMMON_SRC_DIRECTORY}/sdk_inlines.c)

# Link to built libraries
target_link_libraries(LED_SDK LINK_PUBLIC pico_stdlib hardware_gpio)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(LED_SDK 1)
pico_enable_stdio_uart(LED_SDK 1)

# Enable extra build products
pico_add_extra_outputs(LED_SDK)
