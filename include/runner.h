/*
 * RUNNER Run commands on Shell terminal using popen and stores
 * the response in a linked list to be retrieved later on.
 * This is initially created to help me with the development
 * of the xbee ansi C transparent shell terminal.
 * @file: runner.h
 * @author: Anderson Felippe <adfelippe@gmail.com>
 * @date: 14 Aug 2018
 */

#ifndef _RUNNER_H_
#define _RUNNER_H_

#include "list.h"

#define MAX_CMD_LINE_SIZE	255	/* Maximum size of a single line command response.
					   255 characters should be enough. */

int run_system_command(const char* cmd, list *response);
void print_cmd_response(void);

#endif /*_RUNNER_H_*/
