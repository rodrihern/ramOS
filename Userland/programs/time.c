// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "usrlib.h"

int time_main(int argc, char *argv[])
{
	time_info_t info;
	sys_time_info(&info);
	

	printf("%2d:%2d:%2d %2d/%2d/%2d\n", info.hour, info.minutes, info.seconds,
		info.day, info.month, info.year);
	printf("ms since boot: %u\n", sys_ms_elapsed());

	return OK;
}
