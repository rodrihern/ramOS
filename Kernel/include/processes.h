#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "pipes.h"

#define MAX_PROCESSES 64
#define MAX_PROCESS_NAME_LENGTH 32
#define PROCESS_STACK_SIZE (4096 * 2) // 8KB stack
#define MAX_PID (MAX_PROCESSES - 1)
#define KILLED_RET_VALUE -1

#define INIT_PID 0
#define NO_PID -1
#define NO_FD -1

#define OK 0
#define ERROR -1

typedef int (*process_entry_t)(int argc, char **argv);


typedef enum { PS_READY = 0, PS_RUNNING, PS_BLOCKED, PS_TERMINATED } process_status_t;

// Process Control Block
typedef struct pcb {
	int  pid;
	int  parent_pid; // PID del proceso padre (-1 si no tiene)
	char name[MAX_PROCESS_NAME_LENGTH];
	
	process_status_t status;
	uint8_t          priority;           // Prioridad base (0-2, 0 = mayor prioridad)
	uint8_t          effective_priority; // prioridad de aging
	uint64_t         last_tick;

	void *stack_base;    // Base del stack
	void *stack_pointer; // RSP actual (apunta al contexto guardado)
	
	process_entry_t entry;
	int             argc;
	char          **argv;

	// Estadísticas
	int      return_value; // Valor de retorno (para exit)
	int      waiting_on;   // PID que está esperando (-1 si ninguno)

	uint8_t killable; // si false, el proceso no puede ser matado (init/shell)
	uint8_t unblockable;  

	int8_t fd_table[MAX_FDS]; // -1 si no lo tiene abierto, sino el numero al que esta mapeado
} pcb_t;

// Estructura para exponer información de procesos a userland
typedef struct process_info {
	int              pid;
	char             name[MAX_PROCESS_NAME_LENGTH];
	process_status_t status;
	uint8_t          priority;
	int              parent_pid;
	int              read_fd;
	int              write_fd;
	uint64_t         stack_base;
	uint64_t         stack_pointer;
} process_info_t;

typedef struct process_attrs {
	uint8_t read_fd;
	uint8_t write_fd;
	uint8_t priority;
	uint8_t foreground; // 1 = fg, 0 = bg
} process_attrs_t;


// Creación y limpieza (usadas por scheduler)
int create_process(process_entry_t entry, int argc, const char **argv, 
	const char * name, process_attrs_t * attrs);
int init_pcb(pcb_t * p, process_entry_t entry, int argc, const char ** argv, const char * name,
		int parent_pid, uint8_t priority , uint8_t read_fd, uint8_t write_fd, uint8_t killable);
int init_pcb_stack(pcb_t *p);
				 
void free_process_resources(pcb_t *p);

#endif
