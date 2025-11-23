#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include <stddef.h>

#define HEAP_START_ADDRESS 0x600000 // Dirección de inicio del heap
#define HEAP_SIZE 0x2000000         // 32MB de heap

// TAD - Tipo ABStracto de datos (estructura opaca)
typedef struct memory_manager_CDT *memory_manager_ADT;

// Estructura para estadísticas de memoria
typedef struct {
	size_t total_memory;
	size_t used_memory;
	size_t free_memory;
	size_t allocated_blocks;
} mem_info_t;

void *mm_alloc(memory_manager_ADT memory_manager, size_t size);
void mm_free(memory_manager_ADT memory_manager, void *ptr);
void get_memory_info(memory_manager_ADT memory_manager, mem_info_t * buffer);
void init_kernel_memory_manager(void);
memory_manager_ADT get_kernel_memory_manager(void);

#endif 