.syntax unified
.thumb_func

// note that calling bl and bx adds 5 extra cycles every time a function is called
// this could be better optimized later, if needed

/*  Name:           clear_packet_buffer
    Registers Used: r0-r1
    Parameters:     None
    Description:    Clears the packet read buffer.
*/
.global clear_packet_buffer
clear_packet_buffer:
        movs r1, #0

        ldr r0, =PACKET_BUFFER                  // clear the entire buffer at once (assumes the packet buffer is exactly 4 bytes)
        str r1, [r0]

        ldr r0, =PACKET_BITS
        strb r1, [r0]                           // resets the bit counter for the packet to 0

        bx lr                                   // return

/*  Name:           print_packet
    Registers Used: r0-r1
    Parameters:     None
    Description:    Prints the contents of the packet read buffer over UART.
*/
.global print_packet
print_packet:
        push {lr}

        ldr r0, =PACKET_BUFFER                  // load the entire packet buffer into r1 (assumes the packet buffer is exactly 4 bytes)
        ldr r1, [r0]

        ldr r0, =PRINT_PACKET_STRING            // prints the packet buffer using a formatted string
        bl printf

        pop {pc}                                // return

/*  Name:           sleep_for_cycles
    Registers Used: r0-r1
    Parameters:     r0: the number of cycles to delay for
    Description:    Stops thread execution for a number of cycles.
*/
sleep_for_cycles:
        ldr r1, =0xe000e014                     // load the address of the systick reload value register
        str r0, [r1]                            // sets the number of cycles to delay

        movs r0, #5                             // start the systick counter
        ldr r1, =0xe000e010
        str r0, [r1]

sleep_for_cycles_loop:                          // loop until a number of cycles elapses
        ldr r0, [r1]                            // get the systick status
        lsrs r0, 16                             // the bit we want to read is bit 16
        beq sleep_for_cycles_loop               // loop until a number of cycles elapses

        movs r0, #4                             // stop the systick counter
        str r0, [r1]

        ldr r1, =0xe000e018                     // load the address of the systick current value register
        str r0, [r1]                            // reset the systick counter to 0 (writing any value to this register works)

        bx lr                                   // return