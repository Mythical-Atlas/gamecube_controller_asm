/*
 * Derived from the btstack hid_host_demo:
 * Copyright (C) 2017 BlueKitchen GmbH
 *
 * Modifications Copyright (C) 2021-2023 Brian Starkey <stark3y@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
//#include "pico/cyw43_arch.h"
#include "pico/async_context.h"

#include "btstack_run_loop.h"
#include "btstack_config.h"
#include "btstack.h"
#include "classic/sdp_server.h"
#include "hardware/spi.h"

#include "bt_hid.h"

#define MAX_ATTRIBUTE_VALUE_SIZE 512

struct bt_hid_state latest;
mutex_t controller_mutex;

static const char * remote_addr_string = "98:41:5C:B4:83:04";

static bd_addr_t remote_addr;
static bd_addr_t connected_addr;
static btstack_packet_callback_registration_t hci_event_callback_registration;

// SDP
static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];

static uint16_t hid_host_cid = 0;
static bool     hid_host_descriptor_available = false;
static hid_protocol_mode_t hid_host_report_mode = HID_PROTOCOL_MODE_REPORT;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static void hid_host_setup(void){
	// Initialize L2CAP
	l2cap_init();

	sdp_init();

	// Initialize HID Host
	hid_host_init(hid_descriptor_storage, sizeof(hid_descriptor_storage));
	hid_host_register_packet_handler(packet_handler);

	// Allow sniff mode requests by HID device and support role switch
	gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);

	// try to become master on incoming connections
	hci_set_master_slave_policy(HCI_ROLE_MASTER);

	// register for HCI events
	hci_event_callback_registration.callback = &packet_handler;
	hci_add_event_handler(&hci_event_callback_registration);
}

const struct bt_hid_state default_state = {
	.a = false,
    .b = false,
    .x = false,
    .y = false,
    .l = false,
    .r = false,
    .zl = false,
    .zr = false,
    .sl = false,
    .sr = false,
    .minus = false,
    .plus = false,
    .home = false,
    .screenshot = false,
    .dpad = 0x08,
    .left_x = 0x7f,
    .left_y = 0x7f,
    .right_x = 0x7f,
    .right_y = 0x7f
};

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

static void hid_host_handle_interrupt_report(const uint8_t *packet, uint16_t packet_len){
    static struct bt_hid_state last_state = { 0 };

	if(packet_len != 20) return;

	if (
        (packet[0] != 0xef) ||
        (packet[1] != 0x11) ||
        (packet[2] != 0x0c) ||
        (packet[3] != 0x01) ||
        (packet[4] != 0x00) ||
        (packet[5] != 0x0c) ||
        (packet[6] != 0x00) ||
        (packet[7] != 0xa1) ||
        (packet[8] != 0x3f)
    ) return;

	struct input_report_17 *report = (struct input_report_17 *)&packet[0];

    //mutex_enter_blocking(&CONTROLLER_MUTEX_ASM);

    /*latest = (struct bt_hid_state){
        .a =  (report->buttons[0] >> 0) & 1,
        .b =  (report->buttons[0] >> 1) & 1,
        .x =  (report->buttons[0] >> 2) & 1,
        .y =  (report->buttons[0] >> 3) & 1,
        .l =  (report->buttons[0] >> 4) & 1,
        .r =  (report->buttons[0] >> 5) & 1,
        .zl = (report->buttons[0] >> 6) & 1,
        .zr = (report->buttons[0] >> 7) & 1,
        .sl =         (report->buttons[1] >> 2) & 1,
        .sr =         (report->buttons[1] >> 3) & 1,
        .minus =      (report->buttons[1] >> 0) & 1,
        .plus =       (report->buttons[1] >> 1) & 1,
        .home =       (report->buttons[1] >> 4) & 1,
        .screenshot = (report->buttons[1] >> 5) & 1,
        .dpad = report->dpad,
        .left_x = report->left_x,
        .left_y = report->left_y,
        .right_x = report->right_x,
        .right_y = report->right_y,
    };*/

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

    POLL_RESPONSE_START[0] =
        (((report->buttons[1] >> 1) & 1) << 4) + // plus
        (((report->buttons[0] >> 3) & 1) << 3) + // y
        (((report->buttons[0] >> 2) & 1) << 2) + // x
        (((report->buttons[0] >> 1) & 1) << 1) + // b
        ((report->buttons[0] >> 0) & 1); // a
    
    POLL_RESPONSE_START[1] = 0x80 +
        (((report->buttons[0] >> 6) & 1) << 6) + // zl (l)
        (((report->buttons[0] >> 7) & 1) << 5) + // zr (r)
        ((((report->buttons[0] >> 4) & 1) | ((report->buttons[0] >> 5) & 1)) << 4) + // l and r
        dpad_nib;
    POLL_RESPONSE_START[2] = 0x80;//report->left_x;
    POLL_RESPONSE_START[3] = 0x80;//255 - report->left_y;
    POLL_RESPONSE_START[4] = 0x80;//report->right_x;
    POLL_RESPONSE_START[5] = 0x80;//255 - report->right_y;
    POLL_RESPONSE_START[6] = 0x00;//((report->buttons[0] >> 6) & 1) * 255; // zl (analog)
    POLL_RESPONSE_START[7] = 0x00;//((report->buttons[0] >> 7) & 1) * 255; // zr (analog)

    spi_write_blocking(spi_default, POLL_RESPONSE_START, 8);

    //mutex_exit(&CONTROLLER_MUTEX_ASM);
}

static void bt_hid_disconnected(bd_addr_t addr)
{
    /*
	hid_host_cid = 0;
	hid_host_descriptor_available = false;

    mutex_enter_blocking(&CONTROLLER_MUTEX_ASM);

	memcpy(&latest, &default_state, sizeof(latest));

    mutex_exit(&CONTROLLER_MUTEX_ASM);
    */
}

static void try_connect()
{
    printf("Starting hid_host_connect (%s)\n", bd_addr_to_str(remote_addr));

    uint8_t status = hid_host_connect(remote_addr, hid_host_report_mode, &hid_host_cid);
    if (status != ERROR_CODE_SUCCESS)
    {
        printf("hid_host_connect command failed: 0x%02x\n", status);
        bt_hid_disconnected(connected_addr);
        //try_connect();
    }
}

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	UNUSED(channel);
	UNUSED(size);

	uint8_t   event;
	uint8_t   hid_event;
	bd_addr_t event_addr;
	uint8_t   status;
	uint8_t reason;

	if (packet_type != HCI_EVENT_PACKET) {
		return;
	}

	event = hci_event_packet_get_type(packet);
	switch (event) {
	case BTSTACK_EVENT_STATE:
		// On boot, we try a manual connection
		if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
			try_connect();
		}
		break;
	case HCI_EVENT_CONNECTION_COMPLETE:
		status = hci_event_connection_complete_get_status(packet);
		printf("Connection complete: %x\n", status);
		break;
	case HCI_EVENT_DISCONNECTION_COMPLETE:
		status = hci_event_disconnection_complete_get_status(packet);
		reason = hci_event_disconnection_complete_get_reason(packet);
		printf("Disconnection complete: status: %x, reason: %x\n", status, reason);

        bt_hid_disconnected(connected_addr);
        //try_connect();

		break;
	case HCI_EVENT_MAX_SLOTS_CHANGED:
		status = hci_event_max_slots_changed_get_lmp_max_slots(packet);
		//printf("Max slots changed: %x\n", status);
		break;
	case HCI_EVENT_USER_CONFIRMATION_REQUEST:
		//printf("SSP User Confirmation Request: %d\n", little_endian_read_32(packet, 8));
		break;
	case HCI_EVENT_HID_META:
		hid_event = hci_event_hid_meta_get_subevent_code(packet);
		switch (hid_event) {
		case HID_SUBEVENT_CONNECTION_OPENED:
			status = hid_subevent_connection_opened_get_status(packet);
			hid_subevent_connection_opened_get_bd_addr(packet, event_addr);
			if (status != ERROR_CODE_SUCCESS) {
				printf("Connection to %s failed: 0x%02x\n", bd_addr_to_str(event_addr), status);
				bt_hid_disconnected(event_addr);
                //try_connect();
				return;
			}
			hid_host_descriptor_available = false;
			hid_host_cid = hid_subevent_connection_opened_get_hid_cid(packet);
			printf("Connected to %s\n", bd_addr_to_str(event_addr));
			bd_addr_copy(connected_addr, event_addr);
			break;
		case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
			status = hid_subevent_descriptor_available_get_status(packet);
			if (status == ERROR_CODE_SUCCESS){
				hid_host_descriptor_available = true;

                /*
				uint16_t dlen = hid_descriptor_storage_get_descriptor_len(hid_host_cid);
				printf("HID descriptor available. Len: %d\n", dlen);

                printf_hexdump(hid_descriptor_storage_get_descriptor_data(hid_host_cid), dlen);
                */

				// Send FEATURE 0x05, to switch the controller to "full" report mode
				//hid_host_send_get_report(hid_host_cid, HID_REPORT_TYPE_FEATURE, 0x05);
			} else {
				printf("Couldn't process HID Descriptor, status: %d\n", status);
			}
			break;
		case HID_SUBEVENT_REPORT:
            if (hid_host_descriptor_available){
                //if(hci_event_hid_meta_get_subevent_code(packet) == 0x3f)
                //{
                    //printf("len: %i\npacket:\n", size);
                    //printf_hexdump(packet, size);
                //}

                hid_host_handle_interrupt_report(packet, size);

				//hid_host_handle_interrupt_report(hid_subevent_report_get_report(packet), hid_subevent_report_get_report_len(packet));
			} else {
				printf("No hid host descriptor available\n");
				printf_hexdump(hid_subevent_report_get_report(packet), hid_subevent_report_get_report_len(packet));
			}
			break;
		case HID_SUBEVENT_CONNECTION_CLOSED:
			printf("HID connection closed: %s\n", bd_addr_to_str(connected_addr));
			bt_hid_disconnected(connected_addr);
			break;
		default:
			printf("Unknown HID subevent: 0x%x\n", hid_event);
			break;
		}
		break;
	default:
		//printf("Unknown HCI event: 0x%x\n", event);
		break;
	}
}

void bt_main(void) {
	if (cyw43_arch_init()) {
		//printf("Wi-Fi init failed\n");
		return;
	}

	gap_set_security_level(LEVEL_2);

	hid_host_setup();
	sscanf_bd_addr(remote_addr_string, remote_addr);
	bt_hid_disconnected(remote_addr);

	hci_power_control(HCI_POWER_ON);

	btstack_run_loop_execute();
}