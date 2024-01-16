cmake_minimum_required(VERSION 3.14)

# Set project data
set(PROJECT_NAME "RP2040_GAMECUBE_ASM_PROJECT")
set(BUILD_NUMBER "0")

# Set the common code source directory
set(COMMON_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/source/common")

# Set env variable 'PICO_SDK_PATH' to the local Pico SDK
# Comment out the set() if you have a global copy of the
# SDK set and $PICO_SDK_PATH defined in your $PATH
#set(ENV{PICO_SDK_PATH} "${CMAKE_SOURCE_DIR}/pico-sdk")

# Include the Pico SDK
include(pico_sdk_import.cmake)

# Name the project
project(${PROJECT_NAME}
        LANGUAGES C CXX ASM
        DESCRIPTION "RP2040 GameCube Controller Simulator"
)

# Initialise the Pico SDK
pico_sdk_init()

# Include app source code directories
add_subdirectory("${CMAKE_SOURCE_DIR}/source/asm_led_sdk")
add_subdirectory("${CMAKE_SOURCE_DIR}/source/asm_test")
add_subdirectory("${CMAKE_SOURCE_DIR}/source/gc_controller_sim")
add_subdirectory("${CMAKE_SOURCE_DIR}/source/bluetooth_test")