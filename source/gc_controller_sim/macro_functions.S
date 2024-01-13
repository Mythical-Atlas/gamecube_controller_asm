.syntax unified
.thumb_func

// note that calling bl and bx adds 5 extra cycles every time a function is called
// this could be better optimized later, if needed

/*  Name:           clear_packet_buffer
    Registers:      r0-r1
    Description:    Clears the packet read buffer.
*/
.global clear_packet_buffer
clear_packet_buffer:
        ldr r0, =PACKET_BUFFER                  // clear the entire buffer at once (assumes the packet buffer is exactly 4 bytes)
        movs r1, #0
        str r1, [r0]

        bx lr                                   // return

// uses r0-r3
// will be necessary if we go back to a buffer that's not 4 bytes
/*
        ldr r0, =PACKET_BUFFER                  // buffer address
        movs r1, #0                             // offset counter
        movs r2, #0                             // used to store 0x00s in memory
    
clear_packet_buffer_loop:                       // loop until the buffer is cleared
        strb r2, [r0, r1]                       // clear the byte in memory
        adds r1, r1, #1                         // increment counter

        cmp r1, PACKET_BUFFER_SIZE
        bne clear_packet_buffer_loop            // loop until the buffer is cleared

        bx lr                                   // return
*/

/*  Name:           print_packet
    Registers:      r0-r1
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