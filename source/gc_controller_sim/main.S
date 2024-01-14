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
.equ RS_WAITING_FOR_BIT_START,      0
.equ RS_WAITING_FOR_BIT_END_READ,   1
.equ RS_WAITING_FOR_BIT_END_WRITE,  2

// write state machine states
.equ WS_SEND_PROBE_RESPONSE,    0
.equ WS_SEND_ORIGIN_RESPONSE,   1
.equ WS_SEND_POLL_RESPONSE,     2

// timeing
.equ CYCLES_TO_WAIT_BEFORE_LATCH, 150

.global init_controller_sim
init_controller_sim:
        bl stdio_init_all                       // needed to use printf

        movs r0, #28                            // init gpio pin 28
        bl gpio_init

        movs r0, #1                             // disable interrupts
        ldr r1, =0xe000e180                     // unsure how necessary disabling interrupts actually is, but
        str r0, [r1]                            //   just to be on the safe side...

init_read:
        movs r0, #28                            // set gpio pin 28 direction to in
        movs r1, #0
        bl gpio_set_direction

        bl clear_packet_buffer

        movs r0, #RS_WAITING_FOR_BIT_START      // reset READ_STATE
        ldr r1, =READ_STATE
        strb r0, [r1]

loop_read:
        movs r0, #28                            // read gpio pin 28
        bl gpio_get_state

        ldr r1, =READ_STATE                    // load read state
        ldrb r2, [r1]

check_waiting_bit_start:
        cmp r2, #RS_WAITING_FOR_BIT_START
        bne check_waiting_bit_end_read          // wrong state

        cmp r0, #0                              // check pin value
        bne loop_read                           // pin is still high: loop

        // pin is low -> bit has started! start sleeping, then latch bit

        movs r0, #CYCLES_TO_WAIT_BEFORE_LATCH   // set argument
        bl sleep_for_cycles                     // start sleeping
        b latch_bit                             // once sleep is finished, jump to latch_bit

check_waiting_bit_end_read:
        cmp r2, #RS_WAITING_FOR_BIT_END_READ
        bne check_waiting_bit_end_write         // wrong state

        cmp r0, #1                              // check pin value
        bne loop_read                           // pin is still low: loop

        // pin is high -> bit has ended! start a new read cycle (next bit, buffer not cleared)

        movs r0, #RS_WAITING_FOR_BIT_START      // reset READ_STATE
        strb r0, [r1]

        b loop_read

check_waiting_bit_end_write:                    // note: at this point it can be safely assumed that READ_STATE == RS_WAITING_FOR_BIT_END_WRITE
        cmp r0, #1                              // check pin value
        bne loop_read                           // pin is still low: loop

        // pin is high -> bit has ended! start writing
        // note: READ_STATE does not need to be reset here because the
        //   write process will jump to INIT_READ when it finishes

        b init_write

latch_bit:
        movs r0, #28                            // read gpio pin 28
        bl gpio_get_state

        ldr r5, =PACKET_BITS                    // get the byte offset for us to write to
        ldrb r1, [r5]
        lsrs r1, #3                             // using bits 0b00011000

        ldr r2, =PACKET_BUFFER                  // get the current byte
        ldrb r3, [r2, r1]
             
        ldrb r6, [r5]                           // get the bit offset for us to write to
        movs r4, #0b111                         // using bits 0b00000111
        ands r6, r6, r4

        lsls r0, #7                             // set the correct bit position to add to the byte, based on the offset
        lsrs r0, r6

        adds r3, r3, r0                         // store the new byte
        strb r3, [r2, r1]

        ldrb r1, [r5]                           // increment the bit/byte counter
        adds r1, r1, #1
        strb r1, [r5]

check_receieved_probe:       
        ldr r3, [r2]                            // check if the packet buffer contains 0x8000 (0x00 followed by a stop bit)
        ldr r4, =0x8000
        cmp r3, r4
        bne check_received_origin

        // we've recieved a probe!

        movs r1, #0                             // reset the bit/byte counter
        strb r1, [r5]

        bl print_packet
        bl clear_packet_buffer

        movs r0, #WS_SEND_PROBE_RESPONSE        // set write state to send a probe response after end of bit
        ldr r1, =WRITE_STATE
        strb r0, [r1]

        b finish_packet_checks_write

check_received_origin:
        ldr r4, =0x8041                         // check if the packet buffer contains 0x8041 (0x41 followed by a stop bit)
        cmp r3, r4
        beq packet_buffer_is_origin

check_received_poll:
        cmp r1, #25                             // check if we've read 25 bits
        bne finish_packet_checks_read           // jump if less than 25 (it will never be more than 25)

        // we've read 25 bits!

        ldr r4, =0x80000340                     // check if the packet buffer contains 0x40, 0x03, and 0x00 followed by a stop bit
        cmp r3, r4                              // TODO: lower two bits are rumble, and are not taken into account
        beq packet_buffer_is_poll

        b packet_buffer_is_garbage

packet_buffer_is_origin:
        bl print_packet
        bl clear_packet_buffer

        movs r0, #WS_SEND_PROBE_RESPONSE        // set write state to send a probe response after end of bit
        ldr r1, =WRITE_STATE
        strb r0, [r1]

        b finish_packet_checks_write

packet_buffer_is_poll:
        bl print_packet
        bl clear_packet_buffer

        movs r0, #WS_SEND_POLL_RESPONSE         // set write state to send a probe response after end of bit
        ldr r1, =WRITE_STATE
        strb r0, [r1]

        b finish_packet_checks_write

packet_buffer_is_garbage:
        bl print_packet
        bl clear_packet_buffer

        b finish_packet_checks_read

finish_packet_checks_read:
        movs r0, #RS_WAITING_FOR_BIT_END_READ   // set read state to wait, then start reading again
        ldr r1, =READ_STATE
        strb r0, [r1]
        b loop_read

finish_packet_checks_write:
        movs r0, #RS_WAITING_FOR_BIT_END_WRITE  // set read state to wait, then write
        ldr r1, =READ_STATE
        strb r0, [r1]
        b loop_read

init_write:
        b init_read
loop_write:

// used for calling printf to print the packet to uart
PRINT_PACKET_STRING: .asciz "0x%08X\n"

.data

PACKET_BUFFER:  .fill PACKET_BUFFER_SIZE, 1, 0  // reserve memory for reading a packet from the gamecube
PACKET_BITS:    .byte 0                         // the number of bits we've read from the gamecube into the buffer
                                                // this is basically the size of the in-progress packet being read
                                                // these bits: 0b00000111 can be checked to get the current bit offset
                                                // these bits: 0b00011000 can be checked to get the current byte offset

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