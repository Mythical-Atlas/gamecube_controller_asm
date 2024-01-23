#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"

#include "switch_controller.h"
#include "bt_hid.h"
#include "global_defines.h"

const char* remote_addr_string = "98:41:5C:B4:83:04";

const uint8_t report_header[9] = {0xef, 0x11, 0x0c, 0x01, 0x00, 0x0c, 0x00, 0xa1, 0x3f};

const uint8_t dpad_values[9] =
{
    0b1000, // up
    0b1010, // ur
    0b0010, // right
    0b0110, // dr
    0b0100, // down
    0b0101, // dl
    0b0001, // left
    0b1001, // ul
    0b0000  // neutral
};

void handle_controller_input(const uint8_t *packet, uint16_t packet_len)
{
    // make sure we're only handling a specific type of report (the one that has controller data)
	if(packet_len != 20) return;
    if(memcmp(packet, report_header, 9)) return;

	switch_input_data switch_data = get_switch_controller_data(&packet[9]);

#if DEBUG_MODE > 0
    uint8_t old_controller_state[8];
    memcpy(old_controller_state, POLL_RESPONSE, 8);
#endif

    POLL_RESPONSE[0] =
        (switch_data.plus << 4) +
        (switch_data.y << 3) +
        (switch_data.x << 2) +
        (switch_data.b << 1) +
        (switch_data.a);
    
    // note: switch zl and zr become gamecbue l and r, and switch l and r are both used for gamecube z
    POLL_RESPONSE[1] = 0x80 +
        (switch_data.zl << 6) +
        (switch_data.zr << 5) +
        ((switch_data.l | switch_data.r) << 4) +
        switch_data.dpad;

    POLL_RESPONSE[2] = switch_data.left_x;
    POLL_RESPONSE[3] = switch_data.left_y;
    POLL_RESPONSE[4] = switch_data.right_x;
    POLL_RESPONSE[5] = switch_data.right_y;

    // note: because switch controllers don't have analog triggers, the analog portion of the data
    // sent to the gamecube is basically just 'trigger button pressed' = 255
    POLL_RESPONSE[6] = switch_data.zl * 255; // zl (analog)
    POLL_RESPONSE[7] = switch_data.zr * 255; // zr (analog)

#if DEBUG_MODE > 0
    debug_print_input_events(old_controller_state);
#endif
}

switch_input_data get_switch_controller_data(const uint8_t *packet)
{
    switch_input_data input_data;

    input_data.a =    (packet[0] >> 1) & 1;
    input_data.b =    (packet[0] >> 0) & 1;
    input_data.x =    (packet[0] >> 3) & 1;
    input_data.y =    (packet[0] >> 2) & 1;
    input_data.zl =   (packet[0] >> 6) & 1;
    input_data.zr =   (packet[0] >> 7) & 1;
    input_data.l =    (packet[0] >> 4) & 1;
    input_data.r =    (packet[0] >> 5) & 1;
    input_data.plus = (packet[1] >> 1) & 1;

    input_data.dpad = dpad_values[packet[2]];

    input_data.left_x = packet[4];
    input_data.left_y = packet[6];
    input_data.right_x = packet[8];
    input_data.right_y = packet[10];

    modify_joystick(&input_data.left_x, &input_data.left_y);
    modify_joystick(&input_data.right_x, &input_data.right_y);

    return input_data;
}

void modify_joystick(uint8_t* joy_x, uint8_t* joy_y)
{
    // switch y-axis is inverted compared to gamecube
    (*joy_y) = 0xff - (*joy_y);

    // note: this makes the deadband cross-shapes instead of circular
    // this is easily rectified, but because gamecube controllers have notches anyway,
    // it might be best to leave it like this
    if((*joy_x) >= DEAD_RANGE_LOWER && (*joy_x) < DEAD_RANGE_UPPER) (*joy_x) = 0x80;
    if((*joy_y) >= DEAD_RANGE_LOWER && (*joy_y) < DEAD_RANGE_UPPER) (*joy_y) = 0x80;
}

void debug_print_input_events(uint8_t* old_controller_state)
{
    if( (POLL_RESPONSE[0] & (1 << 0)) && !(old_controller_state[0] & (1 << 0))) printf("A Pressed\n");
    if(!(POLL_RESPONSE[0] & (1 << 0)) &&  (old_controller_state[0] & (1 << 0))) printf("A Released\n");
    if( (POLL_RESPONSE[0] & (1 << 1)) && !(old_controller_state[0] & (1 << 1))) printf("B Pressed\n");
    if(!(POLL_RESPONSE[0] & (1 << 1)) &&  (old_controller_state[0] & (1 << 1))) printf("B Released\n");
    if( (POLL_RESPONSE[0] & (1 << 2)) && !(old_controller_state[0] & (1 << 2))) printf("X Pressed\n");
    if(!(POLL_RESPONSE[0] & (1 << 2)) &&  (old_controller_state[0] & (1 << 2))) printf("X Released\n");
    if( (POLL_RESPONSE[0] & (1 << 3)) && !(old_controller_state[0] & (1 << 3))) printf("Y Pressed\n");
    if(!(POLL_RESPONSE[0] & (1 << 3)) &&  (old_controller_state[0] & (1 << 3))) printf("Y Released\n");
    if( (POLL_RESPONSE[1] & (1 << 6)) && !(old_controller_state[1] & (1 << 6))) printf("L Pressed\n");
    if(!(POLL_RESPONSE[1] & (1 << 6)) &&  (old_controller_state[1] & (1 << 6))) printf("L Released\n");
    if( (POLL_RESPONSE[1] & (1 << 5)) && !(old_controller_state[1] & (1 << 5))) printf("R Pressed\n");
    if(!(POLL_RESPONSE[1] & (1 << 5)) &&  (old_controller_state[1] & (1 << 5))) printf("R Released\n");
    if( (POLL_RESPONSE[1] & (1 << 4)) && !(old_controller_state[1] & (1 << 4))) printf("Z Pressed\n");
    if(!(POLL_RESPONSE[1] & (1 << 4)) &&  (old_controller_state[1] & (1 << 4))) printf("Z Released\n");
    if( (POLL_RESPONSE[0] & (1 << 4)) && !(old_controller_state[0] & (1 << 4))) printf("Start Pressed\n");
    if(!(POLL_RESPONSE[0] & (1 << 4)) &&  (old_controller_state[0] & (1 << 4))) printf("Start Released\n");

    if(POLL_RESPONSE[6] != old_controller_state[6]) printf("Analog L: %i\n", POLL_RESPONSE[6]);
    if(POLL_RESPONSE[7] != old_controller_state[7]) printf("Analog R: %i\n", POLL_RESPONSE[7]);

    if((POLL_RESPONSE[1] & 0b1111) != (old_controller_state[1] & 0b1111))
    {
        uint8_t debug_dpad = (POLL_RESPONSE[1] & 0b1111);

        switch(debug_dpad)
        {
        case 0b0000: printf("D-pad Neutral\n"); break;
        case 0b1000: printf("D-pad Up\n"); break;
        case 0b1010: printf("D-pad Up Right\n"); break;
        case 0b0010: printf("D-pad Right\n"); break;
        case 0b0110: printf("D-pad Down Right\n"); break;
        case 0b0100: printf("D-pad Down\n"); break;
        case 0b0101: printf("D-pad Down Left\n"); break;
        case 0b0001: printf("D-pad Left\n"); break;
        case 0b1001: printf("D-pad Up Left\n"); break;
        }
    }

    if(POLL_RESPONSE[2] != old_controller_state[2] || POLL_RESPONSE[3] != old_controller_state[3])
    {
        printf("Left Stick: (%i, %i)\n", POLL_RESPONSE[2], POLL_RESPONSE[3]);
    }
    if(POLL_RESPONSE[4] != old_controller_state[4] || POLL_RESPONSE[5] != old_controller_state[5])
    {
        printf("Right Stick: (%i, %i)\n", POLL_RESPONSE[4], POLL_RESPONSE[5]);
    }
}