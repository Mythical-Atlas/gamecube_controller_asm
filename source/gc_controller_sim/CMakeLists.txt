cmake_minimum_required(VERSION 3.14)

# Include app source code file(s)
# macro files don't need to be included
add_executable(gc_controller_sim
	${CMAKE_SOURCE_DIR}/source/gc_controller_sim/main.c
	${CMAKE_SOURCE_DIR}/source/gc_controller_sim/main.S
	${COMMON_SRC_DIRECTORY}/sdk_inlines.c
)

# Link to built libraries
target_link_libraries(gc_controller_sim LINK_PUBLIC pico_stdlib hardware_gpio)

# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(gc_controller_sim 1)
pico_enable_stdio_uart(gc_controller_sim 1)

# Enable extra build products
pico_add_extra_outputs(gc_controller_sim)
