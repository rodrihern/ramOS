// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "usrlib.h"

#define UNIT_COUNT 4

static void print_converted(uint64_t value) {
	char *units[UNIT_COUNT] = {"B", "KB", "MB", "GB"};
	int unit_idx = 0;
	while (value >= 1024 && unit_idx < UNIT_COUNT-1) {
		value /= 1024;
		unit_idx++;
	}

	printf("%d %s", value, units[unit_idx]);
}

int mem_main(int argc, char *argv[])
{
	if (argc != 0) {
		printf("mem: Invalid number of arguments.\n");
		return -1;
	}

	mem_info_t info;
	sys_mem_info(&info);
	

	double used_percentaje = (info.used_memory / (double) info.free_memory) * 100;
	double free_percentaje = 100 - used_percentaje;

	printf("Total: ");
	print_converted(info.total_memory);
	putchar('\n');

	printf("Used:  ");
	print_converted(info.used_memory);
	printf(" (%f %%)\n", used_percentaje);

	printf("Free:  ");
	print_converted(info.free_memory);
	printf(" (%f %%)\n", free_percentaje);

	printf("Allocated blocks: %d\n", info.allocated_blocks);

	return OK;
}