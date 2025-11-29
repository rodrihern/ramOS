#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>

#define HEAP_START_ADDRESS 0x600000 // Dirección de inicio del heap
#define HEAP_SIZE 0x2000000         // 32MB de heap

// Estructura para estadísticas de memoria
typedef struct {
	uint64_t total_memory;
	uint64_t used_memory;
	uint64_t free_memory;
	uint64_t allocated_blocks;
} mem_info_t;

void init_memory_manager(void *start_address, uint64_t size);
void *mm_alloc(uint64_t size);
void mm_free(void *ptr);
void get_memory_info(mem_info_t *buffer);

#endif 