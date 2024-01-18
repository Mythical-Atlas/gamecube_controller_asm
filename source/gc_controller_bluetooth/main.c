#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bt_hid.h"

extern void controller_sim();

void main()
{	
    stdio_init_all();

    mutex_init(&CONTROLLER_MUTEX_ASM);

	multicore_launch_core1(bt_main);

    //for(;;) {}
	controller_sim();
}