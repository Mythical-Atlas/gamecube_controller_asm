#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"

#include "btstack_run_loop.h"
#include "btstack_config.h"
#include "btstack.h"
#include "classic/sdp_server.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

#include "bt_hid.h"
#include "global_defines.h"
#include "switch_controller.h"

#define MAX_ATTRIBUTE_VALUE_SIZE 512

struct bt_hid_state latest;

static bd_addr_t remote_addr;
static bd_addr_t connected_addr;
static btstack_packet_callback_registration_t hci_event_callback_registration;

static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];

static uint16_t hid_host_cid = 0;
static bool     hid_host_descriptor_available = false;
static hid_protocol_mode_t hid_host_report_mode = HID_PROTOCOL_MODE_REPORT;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

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

static void bt_hid_disconnected(bd_addr_t addr)
{
	hid_host_cid = 0;
	hid_host_descriptor_available = false;

    POLL_RESPONSE[0] = 0x00;
    POLL_RESPONSE[1] = 0x80;
    POLL_RESPONSE[2] = 0x80;
    POLL_RESPONSE[3] = 0x80;
    POLL_RESPONSE[4] = 0x80;
    POLL_RESPONSE[5] = 0x80;
    POLL_RESPONSE[6] = 0x00;
    POLL_RESPONSE[7] = 0x00;
}

static void try_connect()
{
    printf("Starting hid_host_connect (%s)\n", bd_addr_to_str(remote_addr));

    uint8_t status = hid_host_connect(remote_addr, hid_host_report_mode, &hid_host_cid);
    if (status != ERROR_CODE_SUCCESS)
    {
        printf("hid_host_connect command failed: 0x%02x\n", status);
        bt_hid_disconnected(connected_addr);
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	if(packet_type != HCI_EVENT_PACKET) return;

    uint8_t status;
    uint8_t hid_event;

	uint8_t event = hci_event_packet_get_type(packet);
	switch(event)
    {
	case BTSTACK_EVENT_STATE:
		if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING)
        {
			try_connect();
		}
		break;
	case HCI_EVENT_CONNECTION_COMPLETE:
		status = hci_event_connection_complete_get_status(packet);
		printf("Connection complete: %x\n", status);

        if(status == 4)
        {
            try_connect();
        }

		break;
	case HCI_EVENT_DISCONNECTION_COMPLETE:
		status = hci_event_disconnection_complete_get_status(packet);
		uint8_t reason = hci_event_disconnection_complete_get_reason(packet);
		printf("Disconnection complete: status: %x, reason: %x\n", status, reason);

        bt_hid_disconnected(connected_addr);
        try_connect();

		break;
	case HCI_EVENT_HID_META:
		hid_event = hci_event_hid_meta_get_subevent_code(packet);
		switch(hid_event)
        {
		case HID_SUBEVENT_CONNECTION_OPENED:
			status = hid_subevent_connection_opened_get_status(packet);

            bd_addr_t event_addr;
			hid_subevent_connection_opened_get_bd_addr(packet, event_addr);

			if(status != ERROR_CODE_SUCCESS)
            {
				printf("Connection to %s failed: 0x%02x\n", bd_addr_to_str(event_addr), status);
				bt_hid_disconnected(event_addr);
				return;
			}

			hid_host_descriptor_available = false;
			hid_host_cid = hid_subevent_connection_opened_get_hid_cid(packet);

			printf("Connected to %s\n", bd_addr_to_str(event_addr));

			bd_addr_copy(connected_addr, event_addr);
			break;
		case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
			status = hid_subevent_descriptor_available_get_status(packet);
			if(status == ERROR_CODE_SUCCESS)
            {
				hid_host_descriptor_available = true;
			}
			break;
		case HID_SUBEVENT_REPORT:
            if(hid_host_descriptor_available)
            {
                handle_controller_input(packet, size);
			}
			break;
		case HID_SUBEVENT_CONNECTION_CLOSED:
			printf("HID connection closed: %s\n", bd_addr_to_str(connected_addr));
			bt_hid_disconnected(connected_addr);
			break;
		}
		break;
	}
}

void bt_main(void) {
	cyw43_arch_init();
	gap_set_security_level(LEVEL_2);
	hid_host_setup();
	sscanf_bd_addr(remote_addr_string, remote_addr);
	hci_power_control(HCI_POWER_ON);
	btstack_run_loop_execute();
}