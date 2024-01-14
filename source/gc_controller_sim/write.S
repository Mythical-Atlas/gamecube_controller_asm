.syntax unified
.thumb_func

.global init_write
init_write:
        b init_read // TODO: move all read functions to separate file

        movs r0, #28                            // set gpio pin 28 direction to out
        movs r1, #1
        bl gpio_set_direction

        // set the address and counter(?) for the message we need to send

loop_write:
        // loop through every bit of the message at the specified address

        // delaying when necessary

        // 