#include "xbee/serial.h"

enum mode {
	MODE_HOST,
	MODE_CLIENT
} terminal_mode;

void parse_serial_arguments(int argc, char *argv[], xbee_serial_t *serial, int *mode);
