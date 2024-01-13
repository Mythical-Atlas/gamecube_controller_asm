#include "macro_functions.S"

.syntax unified
.thumb_func

.equ PACKET_BUFFER_SIZE,        4               // we should only ever need to read a maximum of 25 bits (3 bytes + a stop bit) from the gamecube
                                                // also, this is a very convenient size for checking the packet (it's the size of a register)

// message sizes
.equ PROBE_RESPONSE_SIZE,       25              // size includes stop bit
.equ ORIGIN_RESPONSE_SIZE,      81              // size includes stop bit
.equ POLL_RESPONSE,             65              // size includes stop bit

// read state machine states
.equ RS_WAITING_FOR_BIT_START, 0
.equ RS_LATCHING_BIT,          1
.equ RS_WAITING_FOR_BIT_END,   2

.global init_controller_sim
init_controller_sim:
        bl stdio_init_all                       // needed to use printf

        movs r0, 28                             // init gpio pin 28
        bl gpio_init

        movs r0, #1                             // disable interrupts
        ldr r1, =0xe000e180                     // unsure how necessary disabling interrupts actually is, but
        str r0, [r1]                            //   just to be on the safe side...

        bl clear_packet_buffer

        // set state? if necessary later

init_read:
        movs r0, 28                             // set gpio pin 28 direction to in
        movs r1, 0
        bl gpio_set_direction
        

PRINT_PACKET_STRING: .asciz "0x%08X\n"

.data

PACKET_BUFFER:  .fill PACKET_BUFFER_SIZE, 1, 0  // reserve memory for reading a packet from the gamecube
PACKET_BITS:    .byte 0                         // the number of bits we've read from the gamecube into the buffer
                                                // this is basically the size of the in-progress packet being read

READ_STATE:     .byte 0                         // current state of packet-reading state machine
WRITE_STATE:    .byte 0                         // what have we sent to the gamecube?

// length: 25 bits (including stop bit)
PROBE_RESPONSE:         .byte 0b00001001        // response
                        .byte 0b00000000
                        .byte 0b00000011
                        .byte 0b10000000        // stop bit

// length: 81 bits (including stop bit)
ORIGIN_RESPONSE:        .byte 0b00000000        // 0, 0, 0, start, y, x, b, a
                        .byte 0b10000000        // 1, l, r, z, d-up, d-down, d-right, d-left
                        .byte 0b10000000        // joy x (128 -> center)
                        .byte 0b10000000        // joy y (128 -> center)
                        .byte 0b10000000        // c x (128 -> center)
                        .byte 0b10000000        // c y (128 -> center)
                        .byte 0b00000000        // left trigger (0 -> off)
                        .byte 0b00000000        // right trigger (0 -> off)
                        .byte 0, 0              // null bytes
                        .byte 0b10000000        // stop bit

// length: 65 bits (including stop bit)
POLL_RESPONSE:          .byte 0b00000000        // 0, 0, 0, start, y, x, b, a
                        .byte 0b10000000        // 1, l, r, z, d-up, d-down, d-right, d-left
                        .byte 0b10000000        // joy x (128 -> center)
                        .byte 0b10000000        // joy y (128 -> center)
                        .byte 0b10000000        // c x (128 -> center)
                        .byte 0b10000000        // c y (128 -> center)
                        .byte 0b00000000        // left trigger (0 -> off)
                        .byte 0b00000000        // right trigger (0 -> off)
                        .byte 0b10000000        // stop bit