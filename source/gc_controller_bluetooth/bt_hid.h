// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 Brian Starkey <stark3y@gmail.com>

// Setup and run the bluetooth stack, will never return
// i.e. start this on Core 1 with multicore_launch_core1()

#include "pico/sync.h"

void bt_main(void);

extern uint8_t POLL_RESPONSE[8];
