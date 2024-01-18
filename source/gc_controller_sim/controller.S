.syntax unified
.thumb_func

.global get_controller_state
get_controller_state:
        push {lr}

        ldr r0, =CONTROLLER_ALLOWED_CYCLES      // cycles for sleeping
        ldr r1, =0xe000e014                     // load the address of the systick reload value register
        str r0, [r1]                            // sets the number of cycles to delay

        movs r0, #5                             // start the systick counter
        ldr r4, =0xe000e010                     // r4: reserved for systick status reg address
        str r0, [r4]

        //ldr r0, =DEBUG_STR // DEBUG: disabled
        //bl printf

        movs r5, #0 // data 1
        movs r6, #0 // data 2
        movs r7, #0 // half byte counter

controller_state_loop:
        movs r0, #10
        bl getchar_timeout_us
        ldr r1, =0xffffffff
        cmp r0, r1
        beq eof_received

        ldr r1, =0xff
        ands r0, r0, r1

        movs r1, r7
        lsrs r1, #2
        bne store_upper_half

store_lower_half:        
        movs r1, r7
        lsls r1, #30 // these two operations are a cheat to mask 0b11 without using another register
        lsrs r1, #27 // cheat to multiply by 8

        lsls r0, r1 // fix
        adds r5, r0

        b skip_store

store_upper_half:        
        movs r1, r7
        lsls r1, #30 // these two operations are a cheat to mask 0b11 without using another register
        lsrs r1, #27 // cheat to multiply by 8

        lsls r0, r1 // fix
        adds r6, r0

skip_store:
        adds r7, #1
        cmp r7, #8
        bne eof_received

        ldr r1, =POLL_RESPONSE_START
        str r5, [r1]
        adds r1, #4
        str r6, [r1]

        movs r5, #0 // data 1
        movs r6, #0 // data 2
        movs r7, #0 // half byte counter

eof_received:
        ldr r0, [r4]                            // get the systick status
        lsrs r0, 16                             // the bit we want to read is bit 16
        beq controller_state_loop               // loop until a number of cycles elapses

        movs r0, #4                             // stop the systick counter
        str r0, [r4]

        

        //ldr r0, =DEBUG_STR2  // DEBUG: disabled
        //bl printf

        ldr r1, =0xe000e018                     // load the address of the systick current value register
        str r0, [r1]                            // reset the systick counter to 0 (writing any value to this register works)

        pop {pc}                                // return