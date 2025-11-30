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
	uint32_t moduleCount = sizeof(moduleAddresses) / sizeof(moduleAddresses[0]);
	uint64_t moduleSizes[moduleCount];

	loadModules(&endOfKernelBinary, moduleAddresses, moduleSizes);
	clearBSS(&bss, &endOfKernel - &bss);

	// Read total memory from the address provided by the bootloader
	uint32_t ram_amount_mib = *((uint32_t *)0x5020);
	uint64_t total_memory_bytes = (uint64_t)ram_amount_mib * 1024 * 1024;

	uint64_t sampleDataModuleSize = moduleSizes[moduleCount - 1];
	void * mm_start = (void*)((uint64_t)sampleDataModuleAddress + sampleDataModuleSize);
    uint64_t mm_size = total_memory_bytes - (uint64_t)mm_start;

	init_memory_manager(mm_start, mm_size);



	load_idt();

	return getStackBase();
}

int main()
{

	init_timer();
	
	init_scheduler();

	init_semaphores();

	init_keyboard_sem();

	vd_show_tty();


	timer_tick(); // esto hace el salto a userland

	return -1;
}
