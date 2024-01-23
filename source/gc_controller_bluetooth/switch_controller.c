#include "switch_controller.h"
#include "bt_hid.h"
#include "global_defines.h"

static void handle_controller_input(const uint8_t *packet, uint16_t packet_len) {
	if(packet_len != 20) return;

	if( (packet[0] != 0xef) ||
        (packet[1] != 0x11) ||
        (packet[2] != 0x0c) ||
        (packet[3] != 0x01) ||
        (packet[4] != 0x00) ||
        (packet[5] != 0x0c) ||
        (packet[6] != 0x00) ||
        (packet[7] != 0xa1) ||
        (packet[8] != 0x3f)    ) return;

	struct input_report_17 *report = (struct input_report_17 *)&packet[0];

    uint8_t dpad_nib = 0b0000; // neutral
    switch(report->dpad)
    {
    case 0: dpad_nib = 0b1000; break; // up
    case 1: dpad_nib = 0b1010; break;
    case 2: dpad_nib = 0b0010; break; // right
    case 3: dpad_nib = 0b0110; break;
    case 4: dpad_nib = 0b0100; break; // down
    case 5: dpad_nib = 0b0101; break;
    case 6: dpad_nib = 0b0001; break; // left
    case 7: dpad_nib = 0b1001; break;
    // 8 = neutral
    }

    uint8_t left_x = report->left_x;
    uint8_t left_y = 0xff - report->left_y;
    uint8_t right_x = report->right_x;
    uint8_t right_y = 0xff - report->right_y;

    if(left_x >= 0x60 && left_x < 0xa0) left_x = 0x80;
    if(left_y >= 0x60 && left_y < 0xa0) left_y = 0x80;
    if(right_x >= 0x60 && right_x < 0xa0) right_x = 0x80;
    if(right_y >= 0x60 && right_y < 0xa0) right_y = 0x80;

#if DEBUG_MODE > 0
    uint8_t old_controller_state[8];
    memcpy(old_controller_state, POLL_RESPONSE, 8);
#endif

    POLL_RESPONSE[0] =
        (((report->buttons[1] >> 1) & 1) << 4) + // plus
        (((report->buttons[0] >> 2) & 1) << 3) + // y
        (((report->buttons[0] >> 3) & 1) << 2) + // x
        (((report->buttons[0] >> 0) & 1) << 1) + // b
        ((report->buttons[0] >> 1) & 1); // a
    
    POLL_RESPONSE[1] = 0x80 +
        (((report->buttons[0] >> 6) & 1) << 6) + // zl (l)
        (((report->buttons[0] >> 7) & 1) << 5) + // zr (r)
        ((((report->buttons[0] >> 4) & 1) | ((report->buttons[0] >> 5) & 1)) << 4) + // l and r
        dpad_nib;

    POLL_RESPONSE[2] = left_x;
    POLL_RESPONSE[3] = left_y;
    POLL_RESPONSE[4] = right_x;
    POLL_RESPONSE[5] = right_y;

    POLL_RESPONSE[6] = ((report->buttons[0] >> 6) & 1) * 255; // zl (analog)
    POLL_RESPONSE[7] = ((report->buttons[0] >> 7) & 1) * 255; // zr (analog)

#if DEBUG_MODE > 0
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
        uint8_t debug_x = POLL_RESPONSE[2];
        uint8_t debug_y = POLL_RESPONSE[3];

             if(debug_x == 0x80 && debug_y == 0x80) printf("Left Stick Neutral\n");
        else if(debug_x == 0x80 && debug_y >  0x80) printf("Left Stick Up\n");
        else if(debug_x  > 0x80 && debug_y >  0x80) printf("Left Stick Up Right\n");
        else if(debug_x  > 0x80 && debug_y == 0x80) printf("Left Stick Right\n");
        else if(debug_x  > 0x80 && debug_y <  0x80) printf("Left Stick Down Right\n");
        else if(debug_x == 0x80 && debug_y <  0x80) printf("Left Stick Down\n");
        else if(debug_x  < 0x80 && debug_y <  0x80) printf("Left Stick Down Left\n");
        else if(debug_x  < 0x80 && debug_y == 0x80) printf("Left Stick Left\n");
        else if(debug_x  < 0x80 && debug_y >  0x80) printf("Left Stick Up Left\n");
    }
    if(POLL_RESPONSE[4] != old_controller_state[4] || POLL_RESPONSE[5] != old_controller_state[5])
    {
        uint8_t debug_x = POLL_RESPONSE[4];
        uint8_t debug_y = POLL_RESPONSE[5];

             if(debug_x == 0x80 && debug_y == 0x80) printf("Right Stick Neutral\n");
        else if(debug_x == 0x80 && debug_y >  0x80) printf("Right Stick Up\n");
        else if(debug_x  > 0x80 && debug_y >  0x80) printf("Right Stick Up Right\n");
        else if(debug_x  > 0x80 && debug_y == 0x80) printf("Right Stick Right\n");
        else if(debug_x  > 0x80 && debug_y <  0x80) printf("Right Stick Down Right\n");
        else if(debug_x == 0x80 && debug_y <  0x80) printf("Right Stick Down\n");
        else if(debug_x  < 0x80 && debug_y <  0x80) printf("Right Stick Down Left\n");
        else if(debug_x  < 0x80 && debug_y == 0x80) printf("Right Stick Left\n");
        else if(debug_x  < 0x80 && debug_y >  0x80) printf("Right Stick Up Left\n");
    }
#endif
}
