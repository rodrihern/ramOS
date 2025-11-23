// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include "keyboard.h"
#include "idt_loader.h"
#include "timer.h"
#include "video.h"
#include "sound.h"
#include "interrupts.h"
#include "memory_manager.h"
#include "processes.h"
#include "scheduler.h"
#include "semaphores.h"
#include "pipes.h"
#include "naiveConsole.h"

extern void timer_tick();

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void *const sampleCodeModuleAddress = (void *)0x400000;
static void *const sampleDataModuleAddress = (void *)0x500000;

typedef int (*EntryPoint)();

void clearBSS(void *bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void *getStackBase()
{
	return (void *)((uint64_t)&endOfKernel + PageSize * 8 // The size of the stack itself, 32KiB
	                - sizeof(uint64_t)                    // Begin at the top of the stack
	);
}

void *initializeKernelBinary()
{
	void *moduleAddresses[] = {sampleCodeModuleAddress, sampleDataModuleAddress};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

	load_idt();

	return getStackBase();
}

int main()
{
	init_kernel_memory_manager();

	init_scheduler();

	init_semaphore_manager();

	init_keyboard_sem();

	vd_enable_textmode();

	init_timer();

	timer_tick(); // esto hace el salto a userland

	return -1;
}
