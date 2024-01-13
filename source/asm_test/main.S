#include "info_macro.S"

.equ    LED_PIN,        25
.equ    GPIO_DIR_IN,    0
.equ    GPIO_DIR_OUT,   1
.equ    PIN_HIGH,       1
.equ    PIN_LOW,        0

.syntax unified
.thumb_func

.global start_blinking_led
start_blinking_led:  
        bl stdio_init_all           // Jump to SDK STDIO initialisation routine
        movs r0, #0x80              // Set the delay period
        lsls r0, #4
        bl sleep_ms                 // Delay to allow the STDIO-over-USB to ready

        movs r0, #LED_PIN           // Set the pin number
        bl gpio_init                // Jump to SDK GPIO initialisation routine

        movs r0, #LED_PIN           // Set the pin number
        movs r1, #GPIO_DIR_OUT      // Set the pin direction
        bl gpio_set_direction       // Jump to SDK pin direction routine

        print_cpu_info              // Jump to the CPU info readout

loop:
        movs r0, #LED_PIN           // Set the pin number
        movs r1, #PIN_HIGH          // Set the pin state
        bl gpio_set_state           // Jump to SDK pin state routine

        ldr r0, DELAY_TIME_MS       // Set the delay period
                                    // NOTE This form assembles as PC offset addressing,
                                    //      so make sure DELAY_TIME_MS is an address in the
                                    //      code space rather not the .data space
        bl sleep_ms                 // Delay

        movs r0, #LED_PIN           // Set the pin number
        movs r1, #PIN_LOW           // Set the pin state
        bl gpio_set_state           // Jump to SDK pin state routine

        ldr r0, DELAY_TIME_MS       // Set the delay period
        bl sleep_ms                 // Delay

        b loop                      // Loop infinitely

.align 4
DELAY_TIME_MS:  .word           500
CPUID:          .word           0xE000ED00          // Memory map address of CPUID register
TEXT0:          .asciz          "CPUID: 0x%08X\n"
TEXT1:          .asciz          "Part:  0x%08X\n"
TEXT2:          .asciz          "Rev:   0x%08X\n"
TEXT3:          .asciz          "Var:   0x%08X\n"
TEXT4:          .asciz          "MATCH\n"