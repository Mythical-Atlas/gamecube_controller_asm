cmake_minimum_required(VERSION 3.14)

set(PICO_BTSTACK_PATH "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/pico-sdk/lib/btstack")
set(PICO_CYW43_SUPPORTED 1)
set(PICO_BOARD pico_w)

set(COMMON_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/source/common")

include(pico_sdk_import.cmake)

project(RP2040_GAMECUBE_ASM_PROJECT
        LANGUAGES C CXX ASM
        DESCRIPTION "RP2040 GameCube Controller Simulator"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory("${CMAKE_SOURCE_DIR}/source/gc_controller_sim")
add_subdirectory("${CMAKE_SOURCE_DIR}/source/gc_controller_bluetooth")