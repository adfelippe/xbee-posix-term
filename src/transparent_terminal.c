/*
 * Origin pieces of code by:
 *
 * Copyright (c) 2007-2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 * 15 Aug 2018
 *
 * This Source Code Form is subject to the terms of GPL v3 license.
 * If a copy of the GPL was not distributed with this file,
 * You can obtain one at https://www.gnu.org/licenses/gpl-3.0.pt-br.html
 *
 * Anderson Felippe <adfelippe@gmail.com>
 * =======================================================================
 */

/*
	Transparent POSIX Terminal over XBee

	This application will create a transparent terminal between two
	XBee nodes connected to a POSIX device with any Shell terminal.
	Application can run in either HOST or CLIENT mode to receive and
	send shell commands.
	HOST will receive a commdn and respond its output to the CLIENT.
	CLIENT will send commands and print their remote output.

	Please refer to the README file in order to obtain further details.
*/

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "xbee/atcmd.h"
#include "xbee/byteorder.h"
#include "xbee/device.h"
#include "xbee/discovery.h"
#include "xbee/transparent_serial.h"
#include "xbee/wpan.h"
#include "wpan/types.h"
#include "wpan/aps.h"
#include "zigbee/zdo.h"
#include "list.h"
#include "runner.h"
// Common code for handling AT command syntax and execution
#include "_atinter.h"
// Common code for handling remote node lists
#include "_nodetable.h"
// Function for parsing serial port settings from command-line arguments
#include "parse_serial_args.h"

// An instance required for the local XBee
xbee_dev_t my_xbee;
// Node ID to be the target
xbee_node_id_t *target = NULL;
// Linked list for the command response
static list cmd_response_list;
// Mode
int mode = MODE_HOST;
// Private prototypes
int send_data(const xbee_node_id_t *node_id, void FAR *data, uint16_t length);


void transparent_dump(const addr64 FAR *ieee, const void FAR *payload,
		      uint16_t length)
{
	xbee_node_id_t *node_id;
	char buffer[ADDR64_STRING_LENGTH];
	char response[MAX_CMD_LINE_SIZE];
	const uint8_t FAR *message = payload;
	uint16_t i, size;

	//printf("%u bytes from ", length);

	node_id = node_by_addr(ieee);

	if (mode == MODE_HOST) {
		if (node_id != NULL) {
			printf( "'%s':\n", node_id->node_info);
		} else {
			printf( "%s:\n", addr64_format(buffer, ieee));
		}
	}

	for (i = 0; i < length && isprint(message[i]); ++i);

	if (mode == MODE_CLIENT) {
		printf("%.*s", length, message);
	} else if (mode == MODE_HOST) {
		// host mode will run local command and transmit
		printf("\t%.*s\n", length, message);
		char terminal_cmd[250] = {0};
		strncpy(terminal_cmd, (char*)message, length);
		printf("terminal_cmd = %s\n", terminal_cmd);
		// Run command
		run_system_command(terminal_cmd, &cmd_response_list);

		size = list_size(&cmd_response_list);

		for (i = 0; i < size; i++) {
			memset(response, '\0', MAX_CMD_LINE_SIZE);
			list_head(&cmd_response_list, (void*)response, true);
			printf("[%d]\t %s", i, response);
			send_data(target, response, strlen(response));
		}

	} else {
		hex_dump( message, length, HEX_DUMP_FLAG_TAB);
	}
}

// function receiving data on transparent serial cluster when ATAO != 0
int transparent_rx(const wpan_envelope_t FAR *envelope, void FAR *context)
{
	transparent_dump(&envelope->ieee_address, envelope->payload,
			 envelope->length);

	return 0;
}

// function receiving data on transparent serial cluster when ATAO=0
int receive_handler(xbee_dev_t *xbee, const void FAR *raw,
		    uint16_t length, void FAR *context)
{
	const xbee_frame_receive_t FAR *rx_frame = raw;

	if (length >= offsetof(xbee_frame_receive_t, payload))
	{
		transparent_dump(&rx_frame->ieee_address, rx_frame->payload,
				 length - offsetof( xbee_frame_receive_t, payload));
	}

	return 0;
}

void node_discovered(xbee_dev_t *xbee, const xbee_node_id_t *rec)
{
	if (rec != NULL)
	{
		node_add(rec);
		xbee_disc_node_id_dump(rec);
	}
}

int node_add_manual(xbee_dev_t *xbee, const char *mac_address)
{
	int error;
	xbee_node_id_t node;

	memset( &node, 0x00, sizeof node);
	error = addr64_parse( &node.ieee_addr_be, mac_address);
	if (error != 0)
	{
		printf("error %d trying to parse '%s' as 64-bit address\n",
				error, mac_address);
	}
	else
	{
		node.network_addr = WPAN_NET_ADDR_UNDEFINED;
		sprintf( node.node_info, "%08" PRIX32, be32toh(node.ieee_addr_be.l[1]));
		node_discovered(xbee, &node);
	}

	return error;
}

int send_data(const xbee_node_id_t *node_id, void FAR *data, uint16_t length)
{
	wpan_envelope_t env;

	wpan_envelope_create(&env, &my_xbee.wpan_dev, &node_id->ieee_addr_be,
			     node_id->network_addr);
	env.payload = data;
	env.length = length;

	return xbee_transparent_serial(&env);
}

int send_string(const xbee_node_id_t *node_id, char FAR *str)
{
	return send_data(node_id, str, strlen( str));
}

int send_test(const xbee_node_id_t *node_id, int bytes)
{
	char buffer[XBEE_MAX_RFPAYLOAD];

	if (bytes < 0 || bytes > XBEE_MAX_RFPAYLOAD)
	{
		printf("Must send between 1 and %u bytes\n", (int) XBEE_MAX_RFPAYLOAD);
		return -EINVAL;
	}

	memset(buffer, 'X', bytes);
	return send_data(node_id, buffer, bytes);
}

/////// use the endpoint table when AO is non-zero

// must be sorted by cluster ID
const wpan_cluster_table_entry_t digi_data_clusters[] =
{
	// transparent serial goes here (cluster 0x0011)
	{ DIGI_CLUST_SERIAL, transparent_rx, NULL,
		WPAN_CLUST_FLAG_INOUT | WPAN_CLUST_FLAG_NOT_ZCL },

	// handle join notifications (cluster 0x0095) when ATAO is not 0
	XBEE_DISC_DIGI_DATA_CLUSTER_ENTRY,

	WPAN_CLUST_ENTRY_LIST_END
};

/// Used to track ZDO transactions in order to match responses to requests
/// (#ZDO_MATCH_DESC_RESP).
wpan_ep_state_t zdo_ep_state = { 0 };

const wpan_endpoint_table_entry_t sample_endpoints[] = {
	ZDO_ENDPOINT(zdo_ep_state),

	// Endpoint/cluster for transparent serial and OTA command cluster
	{	WPAN_ENDPOINT_DIGI_DATA,		// endpoint
		WPAN_PROFILE_DIGI,			// profile ID
		NULL,					// endpoint handler
		NULL,					// ep_state
		0x0000,					// device ID
		0x00,					// version
		digi_data_clusters			// clusters
	},

	{ WPAN_ENDPOINT_END_OF_LIST }
};

////////// end of endpoint table, only necessary if ATAO is not 0
const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
{
	XBEE_FRAME_HANDLE_LOCAL_AT,

	XBEE_FRAME_HANDLE_ATND_RESPONSE,		// for processing ATND responses

	// this entry is for when ATAO is not 0
	XBEE_FRAME_HANDLE_RX_EXPLICIT,		// rx messages via endpoint table

	// next two entries are used when ATAO is 0
	XBEE_FRAME_HANDLE_AO0_NODEID,			// for processing NODEID messages
	{ XBEE_FRAME_RECEIVE, 0, receive_handler, NULL },		// rx messages direct

	XBEE_FRAME_TRANSMIT_STATUS_DEBUG,

	XBEE_FRAME_TABLE_END
};

void print_menu( void)
{
	puts("help                 This list of options.");
	puts("nd                   Initiate node discovery.");
	puts("nd <node id string>  Search for a specific node ID.");
	puts("atXX[=YY]            AT command to get/set an XBee parameter.");
	puts("    Optional value can be decimal (YYY), hex (0xYYYY)"
																" or string \"YYY\"");
	puts("target               Show the list of known targets.");
	puts("target SOME STRING   Set target based on its NI value.");
	puts("mac <mac address>    Manually add target using its MAC address");
	puts("test <num>           Send a packet of <num> bytes to target");
	puts("");
	puts("   All other commands are sent to the current target.");
	puts("");
}

/*
	main

	Initiate communication with the XBee module, then accept AT commands from
	STDIO, pass them to the XBee module and print the result.
*/
int main(int argc, char *argv[])
{
	char cmdstr[250];
	int status;
	int i;
	xbee_serial_t XBEE_SERPORT;

	// Parse terminal arguments
	parse_serial_arguments(argc, argv, &XBEE_SERPORT, &mode);

	if (mode == MODE_HOST) {
		printf("Terminal started in HOST MODE\n");
		printf("This mode is used to receive commands and send back their responses to the CLIENT\n");
	} else {
		printf("Terminal started in CLIENT MODE\n");
		printf("This mode is used to send terminal commands to the HOST and print their responses\n");
	}

	// initialize the serial and device layer for this XBee device
	if (xbee_dev_init(&my_xbee, &XBEE_SERPORT, NULL, NULL)) {
		printf("Failed to initialize device.\n");
		return 0;
	}
	// Initialize the WPAN layer of the XBee device driver.  This layer enables
	// endpoints and clusters, and is required for all ZigBee layers.
	xbee_wpan_init(&my_xbee, sample_endpoints);
	// Register handler to receive Node ID messages
	xbee_disc_add_node_id_handler(&my_xbee, &node_discovered);
	//Init list to store command response
	if (list_new(&cmd_response_list, MAX_CMD_LINE_SIZE, NULL) != 0) {
		printf("Failed to create list to store command response!\n");
	}
	// Initialize the AT Command layer for this XBee device and have the
	// driver query it for basic information (hardware version, firmware version,
	// serial number, IEEE address, etc.)
	xbee_cmd_init_device(&my_xbee);

	printf( "Waiting for driver to query the XBee device...\n");
	do {
		xbee_dev_tick(&my_xbee);
		status = xbee_cmd_query_status( &my_xbee);
	} while (status == -EBUSY);
	if (status)
	{
		printf( "Error %d waiting for query to complete.\n", status);
	}
	// report on the settings
	xbee_dev_dump_settings(&my_xbee, XBEE_DEV_DUMP_FLAG_DEFAULT);

	print_menu();
	xbee_disc_discover_nodes(&my_xbee, NULL);

   while (1)
   {
      while (xbee_readline(cmdstr, sizeof cmdstr) == -EAGAIN)
      {
      	wpan_tick(&my_xbee.wpan_dev);
      	xbee_cmd_tick();
      }

		if (! strcmpi( cmdstr, "help") || ! strcmp( cmdstr, "?"))
		{
			print_menu();
		}
		else if (! strncmpi( cmdstr, "nd", 2))
      {
			// Initiate discovery for a specified node id (as parameter in command
			// or all node IDs.
			if (cmdstr[2] == ' ')
			{
				printf( "Looking for node [%s]...\n", &cmdstr[3]);
				xbee_disc_discover_nodes(&my_xbee, &cmdstr[3]);
			}
			else
			{
				puts( "Discovering nodes...");
				xbee_disc_discover_nodes(&my_xbee, NULL);
			}
      }
      else if (! strcmpi( cmdstr, "quit"))
      {
	      		list_destroy(&cmd_response_list);
			return 0;
		}
      else if (! strncmpi( cmdstr, "AT", 2))
      {
			process_command( &my_xbee, cmdstr);
	   }
	   else if (! strcmpi( cmdstr, "target"))
	   {
	   	node_table_dump();
	   }
	   else if (! strncmpi( cmdstr, "target", 6))
	   {
	   	target = node_by_name( &cmdstr[7]);
	   	if (target == NULL)
	   	{
	   		printf( "couldn't find a target named '%s'\n", &cmdstr[7]);
	   	}
	   	else
	   	{
	   		printf( "target: ");
	   		xbee_disc_node_id_dump( target);
	   	}
	   }
	   else if (! strncmpi( cmdstr, "mac ", 4))
	   {
	   	node_add_manual(&my_xbee, &cmdstr[4]);
	   }
	   else if (! strncmp( cmdstr, "test ", 5))
	   {
	   	int i;
	   	i = (int) strtoul( &cmdstr[5], NULL, 10);
	   	send_test( target, i);
	   }
	   else
	   {
	   	if (target == NULL)
	   	{
	   		puts("you must first select a target with the `target` command");
	   	}
	   	else
	   	{
			if (mode == MODE_CLIENT) {
		   		i = strlen(cmdstr);
		   		printf("sending %u bytes to '%s'\n", i, target->node_info);
		   		send_data(target, cmdstr, i);
			} else {
				printf("< Terminal won't send commands in HOST mode >\n");
			}
	   	}
	   }
   }

   list_destroy(&cmd_response_list);
}
