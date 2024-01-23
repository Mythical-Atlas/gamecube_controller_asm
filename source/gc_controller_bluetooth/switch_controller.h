#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>

#define DEAD_RANGE_LOWER 0x60
#define DEAD_RANGE_UPPER 0xa0

extern const char* remote_addr_string;

typedef struct {
    bool a;
    bool b;
    bool x;
    bool y;
    bool l;
    bool r;
    bool zl;
    bool zr;
    bool plus;
    uint8_t dpad;
    uint8_t left_x;
    uint8_t left_y;
    uint8_t right_x;
    uint8_t right_y;
} switch_input_data;

void handle_controller_input(const uint8_t *packet, uint16_t packet_len);

switch_input_data get_switch_controller_data(const uint8_t *packet);

void modify_joystick(uint8_t* joy_x, uint8_t* joy_y);

void debug_print_input_events(uint8_t* old_controller_state);

#endif