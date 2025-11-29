// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "memory_manager.h"
#include <string.h>
#include <stdbool.h>
#include "naiveConsole.h"

#define MIN_BLOCK_SIZE 32       // Tamaño mínimo de bloque
#define ALIGN_SIZE 8            // Alineación de memoria (8 bytes)
#define MAGIC_NUMBER 0xDEADBEEF // Para detectar corrupción

// Header de cada bloque de memoria. Es una lista doblemente enlazada.
typedef struct mem_block {
	uint64_t          size; // Tamaño del bloque (sin incluir header)
	struct mem_block *next; // Siguiente bloque en la lista
	struct mem_block *prev; // Bloque anterior
	uint8_t           free; // true si está libre, false si está ocupado
	uint32_t          magic; // Número mágico para verificación.  Al liberar (free_memory) se verifica
	                         // que block->magic == MAGIC_NUMBER antes de confiar en el puntero; si
	                         // alguien pasó una dirección que no proviene del gestor (o fue
	                         // sobrescrita), la comparación falla y se ignora la operación
} mem_block;

// Variables globales del Memory Manager
static void      *mm_start_address    = NULL;
static uint64_t   mm_total_size       = 0;
static mem_block *mm_first_block      = NULL;
static uint64_t   mm_allocated_blocks = 0;
static uint64_t   mm_total_allocated  = 0;

// Alinea un tamaño al múltiplo de ALIGN_SIZE
static uint64_t align(uint64_t size)
{
	return (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
}

// Divide un bloque si es muy grande
static void split_block(mem_block *block, uint64_t size)
{
	if (block->size >= size + sizeof(mem_block) + MIN_BLOCK_SIZE) {
		// Crear nuevo bloque con el espacio restante
		mem_block *newBlock = (mem_block *)((char *)block + sizeof(mem_block) + size);
		newBlock->size      = block->size - size - sizeof(mem_block);
		newBlock->free      = true; // ACA
		newBlock->magic     = MAGIC_NUMBER;
		newBlock->next      = block->next;
		newBlock->prev      = block;

		if (block->next != NULL) {
			block->next->prev = newBlock;
		}

		block->next = newBlock;
		block->size = size;
	}
}

static inline void coalesce_next(mem_block *block)
{
	if (block == NULL) {
		return;
	}
	mem_block *next = block->next;
	if (next != NULL && next->free) {
		block->size += sizeof(mem_block) + next->size;
		block->next = next->next;
		if (block->next != NULL) {
			block->next->prev = block;
		}
	}
}

static inline void coalesce_prev(mem_block *block)
{
	if (block == NULL)
		return;
	mem_block *prev = block->prev;
	if (prev != NULL && prev->free) {
		prev->size += sizeof(mem_block) + block->size;
		prev->next = block->next;
		if (block->next != NULL) {
			block->next->prev = prev;
		}
	}
}

// Fusiona bloques libres adyacentes usando funciones auxiliares. TIENEN QUE ESTAR EN ESE ORDEN!
static void coalesce_blocks(mem_block *block)
{
	coalesce_next(block);
	coalesce_prev(block);
}

// Busca el primer bloque libre que tenga el tamaño suficiente (First Fit)
static mem_block *find_free_block(uint64_t size)
{
	mem_block *current = mm_first_block;

	while (current != NULL) {
		if (current->free && current->size >= size && current->magic == MAGIC_NUMBER) {
			return current;
		}
		current = current->next;
	}

	return NULL; // No hay bloque libre suficiente
}

void init_memory_manager(void *start_address, uint64_t size)
{
	// Valida tamaño mínimo para al menos un bloque
	if (start_address == NULL || size < sizeof(mem_block) + MIN_BLOCK_SIZE) {
		return;
	}

	// Inicializar variables globales
	mm_start_address    = start_address;
	mm_total_size       = size;
	mm_allocated_blocks = 0;
	mm_total_allocated  = 0;

	// Crear el primer bloque libre al inicio del área de memoria
	mm_first_block = (mem_block *)start_address;
	mm_first_block->size  = size - sizeof(mem_block);
	mm_first_block->free  = true;
	mm_first_block->next  = NULL;
	mm_first_block->prev  = NULL;
	mm_first_block->magic = MAGIC_NUMBER;
}

void *mm_alloc(uint64_t size)
{
	if (mm_start_address == NULL || size == 0) {
		return NULL;
	}

	// Alinear el tamaño
	size = align(size);

	// Buscar un bloque libre
	mem_block *block = find_free_block(size);

	if (block == NULL) {
		return NULL; // No hay memoria disponible
	}

	// Dividir el bloque si es necesario
	split_block(block, size);

	// Marcar como ocupado
	block->free = false;
	mm_allocated_blocks++;
	mm_total_allocated += block->size;

	// Retornar puntero después del header (donde arranca el espacio utilizable del bloque)
	return (char *)block + sizeof(mem_block);
}

void mm_free(void *ptr)
{
	if (mm_start_address == NULL || ptr == NULL) {
		return;
	}

	// Obtener el bloque desde el puntero
	mem_block *block = (mem_block *)((char *)ptr - sizeof(mem_block));

	// Verificar número mágico
	if (block->magic != MAGIC_NUMBER) {
		// Memoria corrupta o puntero inválido
		return;
	}

	// Verificar que no esté ya libre (double free)
	if (block->free) {
		return;
	}

	// Marcar como libre
	block->free = true;
	mm_allocated_blocks--;
	mm_total_allocated -= block->size;

	// Fusionar con bloques adyacentes
	coalesce_blocks(block);
}

void get_memory_info(mem_info_t *buffer)
{
	if (mm_start_address == NULL || buffer == NULL) {
		return;
	}

	buffer->total_memory = mm_total_size;
	buffer->used_memory  = mm_total_allocated;
	buffer->free_memory  = buffer->total_memory - buffer->used_memory -
	                       (mm_allocated_blocks * sizeof(mem_block));
	buffer->allocated_blocks = mm_allocated_blocks;
}

