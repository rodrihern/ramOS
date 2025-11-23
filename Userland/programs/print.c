// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "usrlib.h"

int print_main(int argc, char *argv[])
{

	if (argc < 1) {
		print_err("Invalid Arguments\n");
		print_err("Use: print <string_to_print>");
		return -1;
	}
	
	while (1) {
		// print(argv[0]);
		for (int i = 0; i < argc; i++) {
			print(argv[i]);
			putchar(' ');
		}
		sys_yield();
	}
}