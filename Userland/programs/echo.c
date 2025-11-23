// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "usrlib.h"

int echo_main(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		if (i > 0) {
			putchar(' ');
		}
		print(argv[i]);
	}
	putchar(EOF);
	return OK;
}