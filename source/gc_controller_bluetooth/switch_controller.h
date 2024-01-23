#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>

#define DEAD_RANGE 0x40

static const char* remote_addr_string = "98:41:5C:B4:83:04";

static void handle_controller_input(const uint8_t *packet, uint16_t packet_len);

struct __attribute__((packed)) input_report_17 {
	uint8_t header[9];
	uint8_t buttons[2];
    uint8_t dpad;
    uint8_t garb0;
    uint8_t left_x;
    uint8_t garb1;
    uint8_t left_y;
    uint8_t garb2;
    uint8_t right_x;
    uint8_t garb3;
    uint8_t right_y;
};

#endif