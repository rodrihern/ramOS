// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "exceptions.h"
#include "keyboard.h"
#include "scheduler.h"
#include "video.h"

static void zero_division();
static void invalid_opcode();
static void excep_handler(char *msg);
extern void return_to_userland();
extern void _hlt();
extern void _sti();

static Exception exceptions[] = {&zero_division, 0, 0, 0, 0, 0, &invalid_opcode};
static char     *message[]    = {"Zero Division Exception", "Invalid Opcode Exception"};

void exception_dispatcher(int exception)
{
	Exception ex = exceptions[exception];
	if (ex != 0) {
		ex();
	}
}

static void excep_handler(char *msg)
{
	vd_print(msg, 0xff0000);
	vd_new_line();
	int pid = sch_get_current_pid();
	sch_kill_process(pid);
	
}

static void zero_division()
{
	excep_handler(message[0]);
}

static void invalid_opcode()
{
	excep_handler(message[1]);
}