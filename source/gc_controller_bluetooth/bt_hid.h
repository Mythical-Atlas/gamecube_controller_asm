// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2023 Brian Starkey <stark3y@gmail.com>

// Setup and run the bluetooth stack, will never return
// i.e. start this on Core 1 with multicore_launch_core1()

#include "pico/sync.h"

void bt_main(void);

struct bt_hid_state {
	bool a;
    bool b;
    bool x;
    bool y;
    bool l;
    bool r;
    bool zl;
    bool zr;
    bool sl;
    bool sr;
    bool minus;
    bool plus;
    bool home;
    bool screenshot;
    uint8_t dpad;
    uint8_t left_x;
    uint8_t left_y;
    uint8_t right_x;
    uint8_t right_y;
};

extern struct bt_hid_state latest;
extern uint8_t POLL_RESPONSE[8];
