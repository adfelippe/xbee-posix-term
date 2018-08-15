/*
 * RUNNER Run commands on Shell terminal using popen and stores
 * the response in a linked list to be retrieved later on.
 * This is initially created to help me with the development
 * of the xbee ansi C transparent shell terminal.
 * @file: runner.c
 * @author: Anderson Felippe <adfelippe@gmail.com>
 * @date: 14 Aug 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "runner.h"

int run_system_command(const char* cmd, list *response)
{
	FILE *in;
	char buffer[MAX_CMD_LINE_SIZE] = {0};


	// Run command on shell
	in = popen(cmd, "r");

	while (1) {
		memset(buffer, 0, MAX_CMD_LINE_SIZE);
                fgets(buffer, MAX_CMD_LINE_SIZE, in);
                if (buffer[0] == 0) {
                        break;
		} else {
			if (list_append(response, buffer) != 0 ) {
            			printf("Failed to append command line response to the list!\n");
				perror("Error");
            			return -1;
			}
		}
        }

	return 0;
}
