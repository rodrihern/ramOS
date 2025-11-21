// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "usrlib.h"

int time_main(int argc, char *argv[])
{
	time_info_t info;
	sys_time(&info);
	

	printf("%d:%d:%d %d/%d/%d\n", info.hour, info.minutes, info.seconds,
		info.day, info.month, info.year);

	return OK;
}
