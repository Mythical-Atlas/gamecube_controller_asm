.syntax unified
.thumb_func

.global init_read
init_read:
        movs r0, #28                            // set gpio pin 28 direction to in
        movs r1, #0
        bl gpio_set_direction

        bl clear_packet_buffer                  // TODO: unsure if these are necessary here, or if they should be removed elsewhere (end of reads/writes)

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