.syntax unified
.thumb_func

.global init_write
init_write:
        movs r5, r0                // address
        movs r6, r1           // size

        movs r0, #28                            // set gpio pin 28 direction to out
        movs r1, #1
        bl gpio_set_direction

        movs r0, #CYCLES_PER_QUARTER_BIT_WRITE  // cycles for sleeping
        ldr r1, =0xe000e014                     // load the address of the systick reload value register
        str r0, [r1]                            // sets the number of cycles to delay

        movs r0, #5                             // start the systick counter
        ldr r7, =0xe000e010
        str r0, [r7]

        movs r4, #0x00                          // byte/bit counter counter

loop_write:
        movs r0, 28                             // set gpio pin 28 to low
        movs r1, 0
        bl gpio_set_state

        // r0: byte index
        // r1: bit index
        // r2: holds current byte of packet
        // r3: used to calculate bit index, then becomes bit mask for current byte
        // r4: reserved for byte/bit counter
        // r5: reserved for packet address
        // r6: reserved for packet size
        // r7: reserved systick status address

        movs r0, r4                             // copy counter
        lsrs r0, #3                             // move bits 0b11111000 to 0b00011111 (byte index)

        ldrb r2, [r5, r0]                       // copy current byte of packet

        movs r1, r4                             // copy counter
        movs r3, #0b111                         // create mask
        ands r1, r1, r3                         // mask to use bits 0b00000111 (bit index)

        movs r3, #0x80                          // used to create bit mask
        lsrs r3, r1                             // bit mask for current bit
        ands r2, r2, r3                         // mask the current byte
        beq write_low                           // current bit is 0
        b write_high                            // current bit is 1


write_finish_bit:
        // r4: reserved for byte/bit counter
        // r6: reserved for packet size

        adds r4, #1
        cmp r4, r6
        bne loop_write

write_finish_packet:
        movs r0, #4                             // stop the systick counter
        str r0, [r7]

        ldr r1, =0xe000e018                     // load the address of the systick current value register
        str r0, [r1]                            // reset the systick counter to 0 (writing any value to this register works)

        b init_read

write_low:
        // r0: used for gpio pin number
        // r1: used for setting gpio pin state
        // r2: used for getting systick status
        // r7: reserved systick status address
write_low_loop_1:                               // this will likely be the quickest loop
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_low_loop_1                    // loop until a number of cycles elapses
write_low_loop_2:
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_low_loop_2                    // loop until a number of cycles elapses
write_low_loop_3:
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_low_loop_3                    // loop until a number of cycles elapses

        movs r0, 28                             // set gpio pin 28 to high
        movs r1, 1
        bl gpio_set_state

write_low_loop_4:
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_low_loop_4                    // loop until a number of cycles elapses

        b write_finish_bit

write_high:
        // r0: used for gpio pin number
        // r1: used for setting gpio pin state
        // r2: used for getting systick status
        // r7: reserved systick status address
write_high_loop_1:                               // this will likely be the quickest loop
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_high_loop_1                    // loop until a number of cycles elapses

        movs r0, 28                             // set gpio pin 28 to high
        movs r1, 1
        bl gpio_set_state

write_high_loop_2:
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_high_loop_2                    // loop until a number of cycles elapses
write_high_loop_3:
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_high_loop_3                    // loop until a number of cycles elapses

write_high_loop_4:
        ldr r2, [r7]                            // get the systick status
        lsrs r2, 16                             // the bit we want to read is bit 16
        beq write_high_loop_4                    // loop until a number of cycles elapses

        b write_finish_bit