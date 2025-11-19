// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "usrlib.h"

#define DEFAULT_DELAY 100

int print_main(int argc, char *argv[])
{

	if (argc < 1) {
		print_err("Use: print <string_to_print> [delay]");
		return -1;
	}

	int delay = DEFAULT_DELAY;
	if (argc >= 2) {
		delay = satoi(argv[1]);
		if (delay <= 0) {
			delay = DEFAULT_DELAY;
		}
	}

	while (1) {
		print(argv[0]);
		sys_sleep(delay);
	}
}